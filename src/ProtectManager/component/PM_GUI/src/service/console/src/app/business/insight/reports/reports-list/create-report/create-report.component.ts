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
import {
  BaseUtilService,
  I18NService,
  DataMapService,
  CommonConsts,
  DataMap,
  ClustersApiService,
  ReportService,
  LANGUAGE,
  RouterUrl
} from 'app/shared';
import {
  FormGroup,
  FormBuilder,
  FormControl,
  AbstractControl
} from '@angular/forms';
import { Component, OnInit } from '@angular/core';
import { Observable, Observer } from 'rxjs';
import {
  isNumber,
  each,
  get,
  filter,
  find,
  size,
  set,
  includes,
  first,
  split,
  uniq,
  now,
  map,
  has,
  isUndefined,
  reject
} from 'lodash';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-create-report',
  templateUrl: './create-report.component.html',
  styleUrls: ['./create-report.component.less']
})
export class CreateReportComponent implements OnInit {
  readonly ONE_DAY = 1000 * 60 * 60 * 24;
  data;
  originAgents;
  localCluster;
  formGroup: FormGroup;
  dataMap = DataMap;
  reportTypeOptions = this.dataMapService
    .toArray('Report_Type')
    .filter(item => {
      item['isLeaf'] = true;
      return includes(
        [
          DataMap.Report_Type.storageSpace.value,
          DataMap.Report_Type.backupJob.value,
          DataMap.Report_Type.recoveryJob.value,
          DataMap.Report_Type.recoveryDrillJob.value,
          DataMap.Report_Type.agentResourceUsed.value,
          DataMap.Report_Type.tapeUsed.value,
          DataMap.Report_Type.resourceUsed.value
        ],
        item.value
      );
    });
  clusterOptions = [];
  formatOptions = this.dataMapService.toArray('Report_Format');
  periodOptions = this.dataMapService
    .toArray('Report_Generated_Period')
    .map(item => {
      item['isLeaf'] = true;
      return item;
    });
  frequencyOptions = [];
  frequencyConfig = this.dataMapService.toArray('reportFrequency').map(item => {
    item['isLeaf'] = true;
    return item;
  });
  showEmailAddress = false;
  showPeriod = this.appUtilsService.isDecouple;
  showPeriodTips = false;
  hideDataSource = false; // 资源使用报表不需要数据源
  isDecouple =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.decouple.value; // e1000

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  rangeErrorTip = {
    invalidEndTime: this.i18n.get('protection_report_timerange_error_label'),
    invalidRange: this.i18n.get('insight_report_max_range_label')
  };
  emailErrorTips = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('insight_report_email_max_size_tips_label'),
    invalidName: this.i18n.get('system_error_email_label'),
    sameEmailError: this.i18n.get('system_same_email_error_label')
  };

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private clusterApiService: ClustersApiService,
    private reportService: ReportService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initReportList();
    this.initForm();
    if (!this.appUtilsService.isDecouple) {
      this.getClusterOptions();
    }
    this.getFrequencyOptions();
  }

  ngAfterViewInit() {
    this.goToPerformance();
  }

  goToPerformance() {
    const domes = document.getElementsByClassName('special-link-performance');
    each(domes, (dom, index) => {
      dom.addEventListener('click', () => {
        window.open(`/console/#${RouterUrl.InsightPerformance}`, '_blank');
      });
    });
  }

  initReportList() {
    // decouple只支持非存储空间报表
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

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      type: new FormControl(
        this.appUtilsService.isDecouple
          ? DataMap.Report_Type.backupJob.value
          : DataMap.Report_Type.storageSpace.value
      ),
      cluster: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      period: new FormControl(''),
      customPeriod: new FormControl([]),
      frequency: new FormControl(''),
      format: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      autoSendEmail: new FormControl(false),
      emailAddress: new FormControl('')
    });
    if (this.isDecouple) {
      this.formGroup.get('cluster').disable();
      this.formGroup
        .get('period')
        .setValidators([this.baseUtilService.VALID.required()]);
      this.formGroup
        .get('frequency')
        .setValidators([this.baseUtilService.VALID.required()]);
    }

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      let updateVal = [];

      each(res, item => {
        if (item.disabled) {
          if (!item.isLeaf) {
            updateVal = [
              ...updateVal,
              ...filter(item.children, { disabled: false })
            ];
          }
        } else {
          updateVal.push(item);
        }
      });

      this.formGroup.get('cluster').setValue(updateVal, { emitEvent: false });
    });

    this.formGroup.get('autoSendEmail').valueChanges.subscribe(res => {
      this.showEmailAddress = res;

      if (res) {
        this.formGroup
          .get('emailAddress')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.vaildEmail()
          ]);
      } else {
        this.formGroup.get('emailAddress').clearValidators();
      }

      this.formGroup.get('emailAddress').updateValueAndValidity();
    });

    this.formGroup.get('type').valueChanges.subscribe(res => {
      this.showPeriod = includes(
        [
          DataMap.Report_Type.resourceProtect.value,
          DataMap.Report_Type.backupJob.value,
          DataMap.Report_Type.recoveryJob.value,
          DataMap.Report_Type.liveMountJob.value,
          DataMap.Report_Type.replicateJob.value,
          DataMap.Report_Type.archiveJob.value,
          DataMap.Report_Type.recoveryDrillJob.value
        ],
        res
      );
      this.showPeriodTips = includes(
        [
          DataMap.Report_Type.clientStatus.value,
          DataMap.Report_Type.storageSpace.value,
          DataMap.Report_Type.quota.value
        ],
        res
      );

      this.hideDataSource = includes(
        [
          DataMap.Report_Type.resourceUsed.value,
          DataMap.Report_Type.agentResourceUsed.value
        ],
        res
      );

      if (this.showPeriod) {
        this.formGroup
          .get('period')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('frequency')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('period').clearValidators();
        this.formGroup.get('frequency').clearValidators();
      }

      if (this.hideDataSource) {
        this.formGroup.get('period').clearValidators();
        this.formGroup.get('frequency').clearValidators();
        this.formGroup.get('cluster').clearValidators();
      } else {
        this.formGroup
          .get('cluster')
          .setValidators([this.baseUtilService.VALID.required()]);
      }

      this.formGroup.get('cluster').updateValueAndValidity();
      this.formGroup.get('period').updateValueAndValidity();
      this.formGroup.get('frequency').updateValueAndValidity();
    });

    this.formGroup.get('period').valueChanges.subscribe(res => {
      if (res === DataMap.Report_Generated_Period.custom.value) {
        this.formGroup
          .get('customPeriod')
          .setValidators([
            this.validCustomPeriod(),
            this.validCurrentTime(),
            this.baseUtilService.VALID.required()
          ]);
      } else {
        this.formGroup.get('customPeriod').clearValidators();
      }
      this.formGroup.get('customPeriod').updateValueAndValidity();
      this.getFrequencyOptions(res);
    });

    this.formGroup.get('customPeriod').valueChanges.subscribe(res => {
      const periodRange = (res[1] - res[0]) / this.ONE_DAY;
      this.getFrequencyOptions(periodRange + 1);
    });
  }

  validCustomPeriod() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!size(control.value)) {
        return;
      }
      return (control.value[1] - control.value[0]) / this.ONE_DAY >= 365
        ? { invalidRange: { value: control.value } }
        : null;
    };
  }

  validCurrentTime() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!size(control.value)) {
        return;
      }
      return control.value[1] > now()
        ? { invalidEndTime: { value: control.value } }
        : null;
    };
  }

  vaildEmail() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return { required: { value: control.value } };
      }

      const emailArr = split(control.value, ',');
      let invalidInfo;

      if (emailArr.length !== uniq(emailArr).length) {
        invalidInfo = { sameEmailError: { value: control.value } };
      }

      if (size(emailArr) > 5) {
        invalidInfo = { invalidMaxLength: { value: control.value } };
      }

      each(emailArr, item => {
        if (!CommonConsts.REGEX.email.test(item)) {
          invalidInfo = { invalidName: { value: control.value } };
        }
      });

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
      this.frequencyOptions = this.frequencyConfig;
    }

    this.formGroup
      .get('frequency')
      .setValue(first(this.frequencyOptions).value);
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
          children: map(get(item, 'nodes', []), node => {
            return {
              ...node,
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
      return;
    });
  }

  getParams() {
    const params = {
      name: this.formGroup.value.name,
      type: this.formGroup.value.type,
      fileFormat: this.formGroup.value.format,
      lang: this.i18n.language === LANGUAGE.EN ? 'EN' : 'zh_CN'
    };

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

    set(params, 'reportDataSources', reportClusterList);

    if (this.hideDataSource || this.isDecouple) {
      // 不需要填写数据源的时候，发送空值
      set(params, 'reportDataSources', []);
    }

    if (
      includes(
        [
          DataMap.Report_Type.backupJob.value,
          DataMap.Report_Type.recoveryJob.value,
          DataMap.Report_Type.recoveryDrillJob.value
        ],
        this.formGroup.value.type
      )
    ) {
      set(params, 'timeRange', this.formGroup.value.period);
      set(params, 'timeUnit', this.formGroup.value.frequency);
    }

    if (
      this.formGroup.value.period ===
      DataMap.Report_Generated_Period.custom.value
    ) {
      let endTime =
        this.formGroup.value.customPeriod[1].getTime() + this.ONE_DAY - 1000;

      if (endTime > now()) {
        endTime = now();
      }

      set(params, 'startTime', this.formGroup.value.customPeriod[0].getTime());
      set(params, 'endTime', endTime);
    }
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();

      this.reportService
        .createReportUsingPOST({
          report: params
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }
}
