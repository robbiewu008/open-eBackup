/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
import { DatePipe } from '@angular/common';
import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup
} from '@angular/forms';
import {
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  RouterUrl,
  SYSTEM_TIME,
  ScheduleReportService
} from 'app/shared';
import { BaseReportParam } from 'app/shared/api/models/base-report-param';
import { TaskRequestParam } from 'app/shared/api/models/task-request-param';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  filter,
  find,
  flatMap,
  get,
  includes,
  intersection,
  isEmpty,
  isUndefined,
  map,
  reject,
  set,
  size,
  some,
  toString,
  uniqueId
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-subscription',
  templateUrl: './create-subscription.component.html',
  styleUrls: ['./create-subscription.component.less'],
  providers: [DatePipe]
})
export class CreateSubscriptionComponent implements OnInit {
  readonly ONE_DAY = 1000 * 60 * 60 * 24;
  data;
  intervalUnit = DataMap.reportGeneratedIntervalUnit;
  daysOfWeek = this.dataMapService
    .toArray('dayOfWeek')
    .filter(item => (item.isLeaf = true));
  monthDaysItems = [];
  formGroup: FormGroup;
  dataMap = DataMap;
  showScopeAndFrequency = false; // 默认是存储空间报表，需要隐藏统计频率和范围
  reportTypeOptions = this.dataMapService
    .toArray('Report_Type')
    .filter(item => {
      return includes(
        [
          DataMap.Report_Type.storageSpace.value,
          DataMap.Report_Type.backupJob.value,
          DataMap.Report_Type.recoveryJob.value,
          DataMap.Report_Type.recoveryDrillJob.value,
          DataMap.Report_Type.agentResourceUsed.value,
          DataMap.Report_Type.resourceUsed.value,
          DataMap.Report_Type.tapeUsed.value
        ],
        item.value
      );
    })
    .filter(item => (item.isLeaf = true));
  showScopeAndFrequencyList = [
    DataMap.Report_Type.backupJob.value,
    DataMap.Report_Type.recoveryJob.value,
    DataMap.Report_Type.recoveryDrillJob.value
  ];
  clusterOptions = [];
  formatOptions = this.dataMapService.toArray('Report_Format');
  periodOptions = this.dataMapService
    .toArray('Report_Generated_Period')
    .filter(item => (item.isLeaf = true));
  frequencyOptions = [];
  frequencyConfig = this.dataMapService
    .toArray('reportFrequency')
    .filter(item => (item.isLeaf = true));

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [40])
  };
  rangeErrorTip = {
    invalidRange: this.i18n.get('insight_report_max_range_label')
  };
  emailErrorTips = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('insight_report_email_max_size_tips_label'),
    invalidName: this.i18n.get('system_error_email_label'),
    sameEmailError: this.i18n.get('system_same_email_error_label')
  };
  daysOfMonthErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidDupInput: this.i18n.get('common_duplicate_input_label')
  };
  MIN_DAY = 1;
  MAX_DAY = 365;

  execIntervalErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [
      this.MIN_DAY,
      this.MAX_DAY
    ])
  };

  customTipsLabel = this.i18n.get('common_valid_rang_label', [
    this.MIN_DAY,
    this.MAX_DAY
  ]);
  isDecouple =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.decouple.value; // e1000

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private datePipe: DatePipe,
    private appUtilsService: AppUtilsService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private clusterApiService: ClustersApiService,
    private scheduleReportService: ScheduleReportService
  ) {}

  ngOnInit() {
    this.initReportList();
    this.initForm();
    this.listenForm();
    this.updateForm();
    if (!this.appUtilsService.isDecouple) {
      this.getClusterOptions();
    }
    this.updateMonthDays();
  }

  setSysTime(formItem) {
    this.appUtilsService.setTimePickerCurrent(formItem);
  }

  initReportList() {
    if (this.appUtilsService.isDecouple) {
      this.reportTypeOptions = reject(this.reportTypeOptions, item =>
        includes(
          [
            DataMap.Report_Type.storageSpace.value,
            DataMap.Report_Type.tapeUsed.value
          ],
          item.value
        )
      );
    }
  }

  ngAfterViewInit() {
    this.goToPerformance();
    this.gotoEmailService();
  }

  goToPerformance() {
    const domes = document.getElementsByClassName('special-link-performance');
    each(domes, (dom, index) => {
      dom.addEventListener('click', () => {
        window.open(`/console/#${RouterUrl.InsightPerformance}`, '_blank');
      });
    });
  }

  gotoEmailService() {
    const domes = document.getElementsByClassName('special-link-email');
    each(domes, (dom, index) => {
      dom.addEventListener('click', () => {
        window.open(
          `/console/#${RouterUrl.SystemSettingsAlarmNotify}`,
          '_blank'
        );
      });
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      policyName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(40)
        ]
      }),
      type: new FormControl([DataMap.Report_Type.storageSpace.value], {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1)
        ]
      }),
      cluster: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      period: new FormControl(
        this.data?.timeRange || DataMap.Report_Generated_Period.week.value,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      customPeriod: new FormControl(1),
      frequency: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      fileFormat: new FormControl(DataMap.Report_Format.xls.value),
      emails: new FormControl([], {
        validators: [this.validEmail(), this.baseUtilService.VALID.maxLength(5)]
      }),
      intervalUnit: new FormControl(this.intervalUnit.day.value), // 天 周 月
      execInterval: new FormControl(1, {
        // 按天执行，多少天执行一次
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(this.MIN_DAY, this.MAX_DAY)
        ]
      }),
      daysOfWeek: new FormControl([], {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1)
        ]
      }), // 按周执行，每周几执行一次
      daysOfMonths: new FormControl([], {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(1)
        ]
      }), // 按月执行，具体一个月哪几天执行
      daysOfMonth: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }), // 界面回显一个月中选中的天数，
      firstExecTime: new FormControl(new Date(), {
        // 首次执行时间
        validators: [this.baseUtilService.VALID.required()]
      }),
      createTime: new FormControl('') // 订阅创建时间，值为点击确认时的时间
    });
    if (this.isDecouple) {
      this.formGroup.get('cluster').disable();
      this.formGroup
        .get('type')
        .setValue([DataMap.Report_Type.resourceUsed.value]); // 改初始值，否则掉校验
    }
    this.formGroup.get('daysOfWeek').disable();
    this.formGroup.get('daysOfMonths').disable();
    this.formGroup.get('daysOfMonth').disable();
    this.changeScopeAndFrequencyStatus(false);
  }

  updateForm() {
    if (!this.data) {
      this.getFrequencyOptions(1);
      return;
    }
    const params = {
      policyName: this.data.policyName,
      type: this.data.type,
      fileFormat: this.data.fileFormat,
      isSendEmail: this.data.isSend,
      emails: this.data.emails,
      intervalUnit: this.data.intervalUnit,
      firstExecTime: new Date(
        this.datePipe.transform(
          this.data.execTime,
          'yyyy-MM-dd HH:mm:ss',
          SYSTEM_TIME.timeZone
        )
      )
    };
    if (this.data.recentDays) {
      set(params, 'customPeriod', this.data.recentDays);
    }
    if (this.data.timeRange) {
      set(params, 'period', this.data.timeRange);
    }
    switch (this.data.intervalUnit) {
      case this.intervalUnit.day.value:
        set(params, 'execInterval', this.data.execInterval);
        break;
      case this.intervalUnit.week.value:
        set(params, 'daysOfWeek', this.data.daysOfWeek);
        break;
      case this.intervalUnit.month.value:
        set(params, 'daysOfMonths', this.data.daysOfMonth);
        break;
    }
    this.formGroup.patchValue(params);
    // 等frequencyOptions处理完后再赋值 避免frequency被提前setValue('')
    defer(() => {
      this.formGroup.get('frequency').setValue(this.data?.timeUnit);
    });
  }

  listenForm() {
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      let updateVal = [];

      each(res, item => {
        if (item.disabled && !item.isLeaf) {
          updateVal = [
            ...updateVal,
            ...filter(item.children, { disabled: false })
          ];
        } else {
          updateVal.push(item);
        }
      });

      this.formGroup.get('cluster').setValue(updateVal, { emitEvent: false });
    });

    this.formGroup.get('daysOfMonths').valueChanges.subscribe(res => {
      this.formGroup
        .get('daysOfMonth')
        .setValue(toString((res || []).sort((a, b) => a - b)));
    });

    this.formGroup.get('period').valueChanges.subscribe(res => {
      this.formGroup.get('frequency').setValue('');
      this.formGroup.get('frequency').markAsTouched();
      this.getFrequencyOptions(res);
    });

    this.formGroup.get('customPeriod').valueChanges.subscribe(res => {
      this.formGroup.get('frequency').setValue('');
      this.getFrequencyOptions(res);
    });

    this.formGroup.get('intervalUnit').valueChanges.subscribe(res => {
      this.formGroup.get('execInterval').disable();
      this.formGroup.get('daysOfWeek').disable();
      this.formGroup.get('daysOfMonths').disable();
      this.formGroup.get('daysOfMonth').disable();
      switch (res) {
        case this.intervalUnit.day.value:
          this.formGroup.get('execInterval').enable();
          break;
        case this.intervalUnit.week.value:
          this.formGroup.get('daysOfWeek').enable();
          break;
        case this.intervalUnit.month.value:
          this.formGroup.get('daysOfMonths').enable();
          this.formGroup.get('daysOfMonth').enable();
          break;
      }
    });

    this.formGroup.get('type').valueChanges.subscribe(res => {
      this.showScopeAndFrequency =
        intersection(this.showScopeAndFrequencyList, res).length > 0;
      this.disabledExternalCluster(res);
      this.changeScopeAndFrequencyStatus(this.showScopeAndFrequency);
    });
  }

  private disabledExternalCluster(res) {
    const isTapeUsed = res.includes(DataMap.Report_Type.tapeUsed.value);
    // 选中的类型有磁带时，需要禁用外部集群
    this.clusterOptions.forEach(item => {
      if (
        isTapeUsed &&
        item.cluster.clusterId !== DataMap.Cluster_Type.local.value
      ) {
        item.disabled = true;
        item.children.forEach(node => (node.disabled = true));
      } else {
        item.disabled = item.children.some(
          node => node.status !== DataMap.Cluster_Status.online.value
        );
        item.children.forEach(
          node =>
            (node.disabled =
              node.status !== DataMap.Cluster_Status.online.value)
        );
      }
    });
    // 需要区分选了父节点和选了子节点两种情况
    this.formGroup
      .get('cluster')
      .setValue(
        reject(this.formGroup.get('cluster').value, item =>
          item?.parent
            ? item.parent.cluster.clusterId !== DataMap.Cluster_Type.local.value
            : item.cluster.clusterId !== DataMap.Cluster_Type.local.value
        )
      );
  }

  private changeScopeAndFrequencyStatus(enable: boolean) {
    this.formGroup
      .get('period')
      [enable ? 'enable' : 'disable']({ emitEvent: false });
    this.formGroup
      .get('customPeriod')
      [enable ? 'enable' : 'disable']({ emitEvent: false });
    this.formGroup
      .get('frequency')
      [enable ? 'enable' : 'disable']({ emitEvent: false });
    if (this.showScopeAndFrequency) {
      if (
        this.formGroup.get('period').value !==
        DataMap.Report_Generated_Period.custom.value
      ) {
        this.getFrequencyOptions(this.formGroup.get('period').value);
      } else {
        this.getFrequencyOptions(this.formGroup.get('customPeriod').value);
      }
    }
  }

  updateMonthDays() {
    const monthDays = Array.from({ length: 31 }, (_, i) => String(i + 1));
    const chunkSize = Math.ceil(monthDays.length / 5);
    this.monthDaysItems = Array.from({ length: 5 }, (_, i) => {
      const start = i * chunkSize;
      const end = start + chunkSize;
      return { key: monthDays.slice(start, end) };
    });
  }

  validEmail() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      let invalidInfo;
      if (size(control.value) > 5) {
        invalidInfo = { invalidMaxLength: { value: control.value } };
      }
      if (some(control.value, item => !CommonConsts.REGEX.email.test(item))) {
        invalidInfo = { invalidName: { value: control.value } };
      }
      return invalidInfo ? invalidInfo : null;
    };
  }

  private getFrequencyOptions(period?) {
    if (period === DataMap.Report_Generated_Period.week.value || period <= 10) {
      this.frequencyOptions = filter(
        this.frequencyConfig,
        item => item.value === DataMap.reportFrequency.oneDay.value
      );
    } else if (
      period === DataMap.Report_Generated_Period.month.value ||
      (period > 10 && period <= 30)
    ) {
      this.frequencyOptions = filter(
        this.frequencyConfig,
        item => item.value === DataMap.reportFrequency.fiveDays.value
      );
    } else if (
      period === DataMap.Report_Generated_Period.threeMonth.value ||
      (period > 30 && period <= 90)
    ) {
      this.frequencyOptions = filter(
        this.frequencyConfig,
        item =>
          item.value === DataMap.reportFrequency.tenDays.value ||
          item.value === DataMap.reportFrequency.thirtyDays.value
      );
    } else if (period > 90 && period <= 365) {
      this.frequencyOptions = filter(
        this.frequencyConfig,
        item => item.value === DataMap.reportFrequency.thirtyDays.value
      );
    } else {
      // 这里限制范围是因为自定义天数如果输入大于365的数，会导致死循环
      if (
        period === DataMap.Report_Generated_Period.custom.value ||
        (period >= 1 && period <= 365)
      ) {
        this.formGroup.get('customPeriod').updateValueAndValidity();
      }
    }
  }

  getClusterOptions() {
    this.clusterApiService.getClusterNodeInfosGET({}).subscribe(res => {
      const clusterArray = [];
      each(res as any, item => {
        clusterArray.push({
          ...item,
          key: item.cluster.remoteEsn,
          value: item.cluster.remoteEsn,
          label: item.cluster.clusterName,
          disabled: !!find(get(item, 'nodes', []), node => {
            return node.status !== DataMap.Cluster_Status.online.value;
          }),
          isLeaf: false,
          uniqueId: uniqueId(),
          children: map(get(item, 'nodes', []), node => {
            return {
              ...node,
              uniqueId: uniqueId(),
              key: node.remoteEsn,
              value: node.remoteEsn,
              label: node.clusterName,
              disabled: node.status !== DataMap.Cluster_Status.online.value,
              isLeaf: true
            };
          })
        });
      });
      this.clusterOptions = clusterArray;
      if (this.data) {
        this.echoClusterData(clusterArray);
      }
      return;
    });
  }

  private echoClusterData = (clusterArray: any[]) => {
    this.formGroup.get('type').setValue(this.data.type);
    let selectedEsnList = [];
    each(clusterArray, item => {
      const target = find(this.data.reportDataSources, {
        clusterId: item.cluster.clusterId
      });
      if (!!target) {
        selectedEsnList.push(...target.esnList);
      }
    });
    const displayNodes = flatMap(clusterArray, item => {
      const target = this.data.reportDataSources.find(
        source => source.clusterId === item.cluster.clusterId
      );
      if (target) {
        // 如果所有的children都被选中了，则直接把整个item选中
        // 其余情况都是把对应的children放进去，注意修改时如果节点离线不需要勾选
        const children = item.children.filter(
          child => selectedEsnList.includes(child.remoteEsn) && !child.disabled
        );
        return children.length === item.children.length ? [item] : children;
      }
      return [];
    });
    this.formGroup.get('cluster').setValue(displayNodes);
  };

  getParams(): TaskRequestParam {
    const formValue = this.formGroup.value;
    const reportParam: BaseReportParam = {
      name: formValue.policyName,
      type: formValue.type,
      fileFormat: formValue.fileFormat,
      timeRange: formValue.period,
      timeUnit: formValue.frequency,
      startTime: 0,
      lang: this.i18n.isEn ? 'EN' : 'zh_CN',
      emails: formValue.emails,
      isSend: !isEmpty(formValue.emails),
      reportDataSources: []
    };
    const params: TaskRequestParam = {
      policyName: reportParam.name,
      policyType: 'PERIOD',
      execTime: this.appUtilsService.toSystemTimeLong(
        this.formGroup.get('firstExecTime').value
      ),
      intervalUnit: formValue.intervalUnit,
      createTime: this.appUtilsService.toSystemTimeLong(new Date()),
      reportParam: reportParam
    };
    if (this.data) {
      assign(params, {
        policyId: this.data.policyId
      });
    }
    switch (formValue.intervalUnit) {
      case this.intervalUnit.day.value:
        set(params, 'execInterval', Number(formValue.execInterval));
        break;
      case this.intervalUnit.week.value:
        set(params, 'daysOfWeek', map(formValue.daysOfWeek, String));
        break;
      case this.intervalUnit.month.value:
        set(params, 'daysOfMonth', map(formValue.daysOfMonths));
        break;
    }
    if (formValue.period === DataMap.Report_Generated_Period.custom.value) {
      set(reportParam, 'isCustomized', true);
      set(reportParam, 'recentDays', formValue.customPeriod);
    }

    const reportClusterList = [];

    each(this.formGroup.value.cluster, item => {
      if (!get(item, 'isLeaf')) {
        reportClusterList.push({
          clusterId: item.cluster.clusterId,
          esnList: map(item.children, child => child.remoteEsn)
        });
      } else {
        const cluster = find(reportClusterList, {
          clusterId: item.parent?.cluster?.clusterId
        });

        if (isUndefined(cluster)) {
          reportClusterList.push({
            clusterId: item.parent?.cluster?.clusterId,
            esnList: [item.remoteEsn]
          });
        } else {
          cluster.esnList.push(item.remoteEsn);
        }
      }
    });

    set(reportParam, 'reportDataSources', reportClusterList);

    if (this.isDecouple) {
      // 不需要填写数据源的时候，发送空值
      set(reportParam, 'reportDataSources', []);
    }

    if (this.showScopeAndFrequency) {
      set(reportParam, 'timeRange', this.formGroup.value.period);
      set(reportParam, 'timeUnit', this.formGroup.value.frequency);
    }
    return params;
  }

  getCreateOrModifyObservable(params: TaskRequestParam) {
    if (this.data) {
      return this.scheduleReportService.ModifyReportSubscriptionPolicyUsingPut({
        ModifyReportSubscriptionPolicyUsingPutRequestBody: params
      });
    } else {
      return this.scheduleReportService.CreateReportSubscriptionPolicyUsingPost(
        {
          CreateReportSubscriptionPolicyUsingPostRequestBody: params
        }
      );
    }
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params: TaskRequestParam = this.getParams();
      if (this.formGroup.invalid) {
        return;
      }
      this.getCreateOrModifyObservable(params).subscribe({
        next: res => {
          observer.next();
          observer.complete();
        },
        error: err => {
          observer.error(err);
          observer.complete();
        }
      });
    });
  }
}
