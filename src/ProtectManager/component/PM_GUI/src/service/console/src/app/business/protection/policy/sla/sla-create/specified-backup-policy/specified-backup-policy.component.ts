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
import { FormBuilder, FormGroup } from '@angular/forms';
import { MessageService, ModalRef } from '@iux/live';
import {
  ApplicationType,
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  DaysOfType,
  I18NService,
  PolicyAction,
  PolicyType,
  ProjectedObjectApiService,
  ProtectedResourceApiService,
  ProtectResourceAction,
  RetentionType,
  ScheduleTrigger,
  SLA_BACKUP_NAME,
  AntiRansomwarePolicyApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { SlaValidatorService } from 'app/shared/services/sla-validator.service';
import {
  assign,
  each,
  filter,
  find,
  findIndex,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isUndefined,
  map as _map,
  map,
  omit,
  pick,
  set,
  size,
  some
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-specified-backup-policy',
  templateUrl: './specified-backup-policy.component.html',
  styleUrls: ['./specified-backup-policy.component.less'],
  providers: [DatePipe]
})
export class SpecifiedBackupPolicyComponent implements OnInit {
  sla;
  action;
  backupData;
  activeIndex;
  specialResource = false;
  specialResourceTips = '';
  archvieDataList;
  replicationData;
  applicationData;
  formGroup: FormGroup;
  applicationType = ApplicationType;
  protectResourceAction = ProtectResourceAction;
  isUsed: boolean;
  _includes = includes;
  hasOpenGauss = []; // 用于判断opengauss已绑定的资源
  isCyber = includes(
    [
      DataMap.Deploy_Type.cyberengine.value,
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cloudbackup.value
    ],
    this.i18n.get('deploy_type')
  );
  isWormData = false; // 判断关联资源是否存在防勒索界面设置的worm
  resourceList = [];
  isBasicDisk = false;
  constructor(
    public baseUtilService: BaseUtilService,
    public i18n: I18NService,
    public fb: FormBuilder,
    public modal: ModalRef,
    public datePipe: DatePipe,
    public dataMapService: DataMapService,
    private messageService: MessageService,
    private slaValidatorService: SlaValidatorService,
    private projectedObjectApiService: ProjectedObjectApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    public appUtilsService: AppUtilsService,
    private antiRansomwarePolicyApiService: AntiRansomwarePolicyApiService
  ) {}

  ngOnInit() {
    this.isUsed = !!this.sla?.resource_count;
    assign(
      this.backupData,
      {
        applicationData: this.applicationData
      },
      {}
    );
    this.formGroup = this.fb.group({});
    setTimeout(() => {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    }, 0);

    if (
      includes(
        [
          ApplicationType.OpenGauss,
          ApplicationType.Dameng,
          ApplicationType.Fileset,
          ApplicationType.NASShare,
          ApplicationType.MongoDB,
          ApplicationType.Volume,
          ApplicationType.ObjectStorage
        ],
        this.applicationData
      ) &&
      get(this.sla, 'uuid')
    ) {
      this.getSlaResource();
    }
    if (!this.isCyber && !isEmpty(this.sla?.uuid)) {
      this.getResourceList();
    }
  }

  // 修改SLA时的特殊场景处理
  getSlaResource() {
    const params = {
      slaId: get(this.sla, 'uuid'),
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      subType: [
        this.applicationData === ApplicationType.Dameng
          ? DataMap.Resource_Type.Dameng_cluster.value
          : this.applicationData === ApplicationType.NASShare
          ? DataMap.Resource_Type.NASShare.value
          : this.applicationData === ApplicationType.ObjectStorage
          ? DataMap.Resource_Type.ObjectSet.value
          : DataMap.Resource_Type.fileset.value
      ]
    };

    if (this.applicationData === ApplicationType.OpenGauss) {
      params.subType = [
        DataMap.Resource_Type.OpenGauss_database.value,
        DataMap.Resource_Type.OpenGauss_instance.value
      ];
    }

    this.projectedObjectApiService
      .pageQueryV1ProtectedObjectsGet(params as any)
      .subscribe(res => {
        if (!!size(res.items)) {
          this.specialResource = true;
          if (this.applicationData === ApplicationType.OpenGauss) {
            this.getOpenGaussValid(res.items);
          }
          this.specialResourceTips =
            this.applicationData === ApplicationType.Dameng
              ? this.i18n.get('protection_dameng_sla_tips_vaild_label')
              : this.applicationData === ApplicationType.OpenGauss
              ? ''
              : this.i18n.get('protection_fileset_sla_tips_label');
        } else {
          this.specialResource = false;
        }
      });
  }

  getResourceList(param?) {
    const params = param || {
      slaId: this.sla.uuid,
      pageNo: CommonConsts.PAGE_START,
      pageSize: 100
    };
    this.projectedObjectApiService
      .pageQueryV1ProtectedObjectsGet(params)
      .subscribe(res => {
        this.resourceList = [...this.resourceList, ...res.items];
        this.resourceList = this.resourceList.map(e => {
          return e.resource_id;
        });
        this.getDataWormStatus(this.resourceList);
      });
  }

  getDataWormStatus(data) {
    const resourceParams = {
      resourceIds: data
    };
    if (resourceParams.resourceIds.length > 0) {
      this.antiRansomwarePolicyApiService
        .ShowAntiRansomwarePolicies(resourceParams)
        .subscribe(res => {
          this.isWormData = !!find(res.records, item => item.schedule.setWorm);
        });
    }
  }

  getOpenGaussValid(items) {
    // openGauss由于日志备份只有使用panwei的实例才支持，数据库和其他实例需要做判断
    this.protectedResourceApiService
      .ListResources({
        pageNo: 0,
        pageSize: items.length,
        conditions: JSON.stringify({
          uuid: map(items, val => val.resource_id)
        })
      })
      .subscribe(res => {
        if (
          some(
            res.records,
            item =>
              item.subType === DataMap.Resource_Type.OpenGauss_database.value
          )
        ) {
          this.hasOpenGauss.push(
            DataMap.Resource_Type.OpenGauss_database.value
          );
        }
        if (
          some(
            res.records,
            item =>
              item.subType === DataMap.Resource_Type.OpenGauss_instance.value &&
              !item.extendInfo.clusterVersion.includes('PanWeiDB')
          )
        ) {
          this.hasOpenGauss.push(
            DataMap.Resource_Type.OpenGauss_instance.value
          );
        }

        this.specialResource = !!this.hasOpenGauss.length;

        if (
          this.hasOpenGauss.includes(
            DataMap.Resource_Type.OpenGauss_database.value
          )
        ) {
          this.specialResourceTips = this.i18n.get(
            'protection_opengauss_sla_tips_vaild_label'
          );
        } else if (!!this.hasOpenGauss.length) {
          this.specialResourceTips = this.i18n.get(
            'protection_unsupport_backup_policy_label',
            [
              this.i18n.get(
                'protection_opengauss_sla_instance_tips_valid_label'
              ),
              '',
              this.i18n.get('common_log_backup_label')
            ]
          );
        }
      });
  }

  validFormArray() {
    const errorMsgs = [];
    const backupTeams = this.formGroup.value.backupTeams;
    const logItem = find(backupTeams, { action: PolicyAction.LOG });
    const diffItemArr = map(
      filter(backupTeams, item => item.action === PolicyAction.DIFFERENCE),
      item => {
        return {
          firstBackupTime: this.firstBackupTime(item),
          item
        };
      }
    );
    const fullItemArr = map(
      filter(backupTeams, item => item.action === PolicyAction.FULL),
      item => {
        return {
          firstBackupTime: this.firstBackupTime(item),
          item
        };
      }
    );
    const incrItemArr = map(
      filter(backupTeams, item => item.action === PolicyAction.INCREMENT),
      item => {
        return {
          firstBackupTime: this.firstBackupTime(item),
          item
        };
      }
    );
    const diffItem = size(diffItemArr)
      ? first(
          diffItemArr.sort(
            (a, b) =>
              Date.parse(a.firstBackupTime) - Date.parse(b.firstBackupTime)
          )
        ).item
      : null;
    const fullItem = size(fullItemArr)
      ? first(
          fullItemArr.sort(
            (a, b) =>
              Date.parse(a.firstBackupTime) - Date.parse(b.firstBackupTime)
          )
        ).item
      : null;
    const incrItem = size(incrItemArr)
      ? first(
          incrItemArr.sort(
            (a, b) =>
              Date.parse(a.firstBackupTime) - Date.parse(b.firstBackupTime)
          )
        ).item
      : null;

    for (const item of backupTeams) {
      if (item.trigger_action === DaysOfType.DaysOfMinute) {
        item.interval_unit = DataMap.Interval_Unit.minute.value;
      } else if (item.trigger_action === DaysOfType.DaysOfDay) {
        item.interval_unit = DataMap.Interval_Unit.day.value;
      } else {
        item.interval_unit = DataMap.Interval_Unit.hour.value;
      }
      const invalidIntervalResult = this.slaValidatorService.validBackupInterval(
        'interval',
        'interval_unit',
        'retention_duration',
        'duration_unit',
        item
      );
      if (invalidIntervalResult) {
        errorMsgs.push(invalidIntervalResult);
        this.messageService.error(
          this.i18n.get(invalidIntervalResult, [SLA_BACKUP_NAME[item.action]]),
          {
            lvMessageKey: 'lvMsg_sla_valid_error',
            lvShowCloseButton: true
          }
        );
        break;
      }
    }
    return !size(errorMsgs);
  }

  firstBackupTime(item) {
    let firstBackupTime;
    // 周期性
    if (
      item['start_time'] &&
      item['window_start'] &&
      includes(
        [DaysOfType.DaysOfDay, DaysOfType.DaysOfHour, DaysOfType.DaysOfMinute],
        item['trigger_action']
      )
    ) {
      firstBackupTime =
        `${item['start_time'].getFullYear()}` +
        '/' +
        (+`${item['start_time'].getMonth()}` + 1) +
        '/' +
        `${item['start_time'].getDate()}` +
        ' ' +
        `${item['window_start'].getHours()}` +
        ':' +
        `${item['window_start'].getMinutes()}` +
        ':' +
        `${item['window_start'].getSeconds()}`;
    }

    // 指定时间
    if (
      item['specified_window_start'] &&
      includes(
        [DaysOfType.DaysOfYear, DaysOfType.DaysOfMonth, DaysOfType.DaysOfWeek],
        item['trigger_action']
      )
    ) {
      const sysTime = new Date();
      let year = sysTime.getFullYear();
      let month = sysTime.getMonth();
      let date = sysTime.getDate();

      if (item['trigger_action'] === 'year') {
        month = item['days_of_year'].getMonth();
        date = item['days_of_year'].getDate();

        // 选择同一天的情况
        if (month === sysTime.getMonth() && date === sysTime.getDate()) {
          if (item['specified_window_start'] <= sysTime) {
            year++;
          }
        } else {
          if (month < sysTime.getMonth()) {
            year++;
          } else if (month === sysTime.getMonth()) {
            if (date < sysTime.getDate()) {
              year++;
            }
          }
        }
      } else if (item['trigger_action'] === 'month') {
        if (findIndex(item['days_of_months'], val => val === date) !== -1) {
          // 选择同一天的情况
          if (item['specified_window_start'] <= sysTime) {
            const idx = findIndex(
              item['days_of_months'],
              (val: any) => val > date
            );

            if (idx === -1) {
              date = first(item['days_of_months']);
              month++;
            } else {
              date = item['days_of_months'][idx];
            }
          }
        } else {
          const idx = findIndex(
            item['days_of_months'],
            (val: any) => val > date
          );

          if (idx === -1) {
            date = first(item['days_of_months']);
            month++;
          } else {
            date = item['days_of_months'][idx];
          }
        }
      } else if (item['trigger_action'] === 'week') {
        const weekVal = {
          mon: 1,
          tue: 2,
          wed: 3,
          thu: 4,
          fri: 5,
          sat: 6,
          sun: 0
        };
        const today = sysTime.getDay();
        let backupDate;

        if (
          findIndex(
            item['days_of_week'],
            val => weekVal[val as string] === today
          ) !== -1
        ) {
          // 选择同一天的情况
          if (item['specified_window_start'] <= sysTime) {
            const idx = findIndex(
              item['days_of_week'],
              val => weekVal[val as string] > today
            );

            idx === -1
              ? (backupDate = weekVal[first(item['days_of_week']) as string])
              : (backupDate = weekVal[item['days_of_week'][idx]]);

            if (today <= backupDate) {
              date += backupDate - today;
            } else {
              date += 7 + backupDate - today;
            }
          }
        } else {
          const idx = findIndex(
            item['days_of_week'],
            val => weekVal[val as string] > today
          );

          idx === -1
            ? (backupDate = weekVal[first(item['days_of_week']) as string])
            : (backupDate = weekVal[item['days_of_week'][idx]]);

          if (today <= backupDate) {
            date += backupDate - today;
          } else {
            date += 7 + backupDate - today;
          }
        }
      }

      firstBackupTime =
        `${year}` +
        '/' +
        (+`${month}` + 1) +
        '/' +
        `${date}` +
        ' ' +
        `${item['specified_window_start'].getHours()}` +
        ':' +
        `${item['specified_window_start'].getMinutes()}` +
        ':' +
        `${item['specified_window_start'].getSeconds()}`;
    }

    return firstBackupTime;
  }

  getBackupParams() {
    const backupTeams = [];
    each(this.formGroup.value.backupTeams, (item, index) => {
      backupTeams.push(this.getParams(item));
    });
    return backupTeams;
  }

  getParams(item) {
    if (
      [
        PolicyAction.FULL,
        PolicyAction.INCREMENT,
        PolicyAction.DIFFERENCE,
        PolicyAction.SNAPSHOT,
        PolicyAction.PERMANENT
      ].includes(item.action)
    ) {
      return {
        ...this.getFullIncrementParams(item),
        ...this.getExtParameters(item)
      };
    }

    if (item.action === PolicyAction.LOG) {
      return {
        ...this.getLogParams(item),
        ...this.getExtParameters(item)
      };
    }

    return {};
  }

  getFullIncrementParams(item) {
    const params = {
      uuid: item.uuid,
      name: item.name,
      type: PolicyType.BACKUP,
      action: item.action,
      worm_validity_type: item.worm_switch ? item.worm_validity_type : 0,
      retention: {
        retention_type:
          DataMap.Interval_Unit.persistent.value === item.duration_unit
            ? RetentionType.PERMANENTLY_RETAINED
            : RetentionType.TEMPORARY_RESERVATION,
        retention_duration: +item.retention_duration,
        duration_unit: item.duration_unit,
        worm_retention_duration: item.worm_switch
          ? item.worm_validity_type === 1
            ? item.retention_duration
            : item.worm_specified_retention_duration
          : null,
        worm_duration_unit: item.worm_switch
          ? item.worm_validity_type === 1
            ? item.duration_unit
            : item.worm_specified_duration_unit
          : null
      },
      schedule: {
        trigger: includes(
          [
            DaysOfType.DaysOfDay,
            DaysOfType.DaysOfHour,
            DaysOfType.DaysOfMinute
          ],
          item.trigger_action
        )
          ? ScheduleTrigger.PERIOD_EXECUTE
          : ScheduleTrigger.SPECIFIED_TIME,
        interval: +item.interval,
        interval_unit: includes(
          [
            DaysOfType.DaysOfDay,
            DaysOfType.DaysOfHour,
            DaysOfType.DaysOfMinute
          ],
          item.trigger_action
        )
          ? item.trigger_action === DaysOfType.DaysOfDay
            ? DataMap.Interval_Unit.day.value
            : item.trigger_action === DaysOfType.DaysOfHour
            ? DataMap.Interval_Unit.hour.value
            : DataMap.Interval_Unit.minute.value
          : DataMap.Interval_Unit.day.value,
        start_time: this.datePipe.transform(item.start_time, 'yyyy-MM-dd'),
        window_start: this.datePipe.transform(item.window_start, 'HH:mm:ss'),
        window_end: this.datePipe.transform(item.window_end, 'HH:mm:ss')
      } as any
    };

    this.getSpecifiedTimeParams(item, params);

    if (
      RetentionType.PERMANENTLY_RETAINED === params.retention.retention_type
    ) {
      delete params.retention.retention_duration;
      delete params.retention.duration_unit;
    }

    if (
      item.action === PolicyAction.INCREMENT &&
      includes(
        [
          ApplicationType.Fileset,
          ApplicationType.NASShare,
          ApplicationType.Volume,
          ApplicationType.ObjectStorage
        ],
        this.applicationData
      ) &&
      !!item.permanentBackup
    ) {
      set(params, 'action', PolicyAction.PERMANENT);
    }
    if (this.isCyber) {
      delete params.retention.worm_retention_duration;
      delete params.retention.worm_duration_unit;
      delete params.worm_validity_type;
    }
    return params;
  }

  getLogParams(item) {
    const params = {
      uuid: item.uuid,
      name: item.name,
      type: PolicyType.BACKUP,
      action: item.action,
      worm_validity_type: item.worm_switch ? item.worm_validity_type : 0,
      retention: {
        retention_type:
          DataMap.Interval_Unit.persistent.value === item.duration_unit
            ? RetentionType.PERMANENTLY_RETAINED
            : RetentionType.TEMPORARY_RESERVATION,
        retention_duration: [
          ApplicationType.HBase,
          ApplicationType.SQLServer
        ].includes(this.applicationData)
          ? null
          : +item.retention_duration,
        duration_unit: item.duration_unit,
        worm_retention_duration: item.worm_switch
          ? item.worm_validity_type === 1
            ? item.retention_duration
            : item.worm_specified_retention_duration
          : null,
        worm_duration_unit: item.worm_switch
          ? item.worm_validity_type === 1
            ? item.duration_unit
            : item.worm_specified_duration_unit
          : null
      },
      schedule: {
        trigger: ScheduleTrigger.PERIOD_EXECUTE,
        interval: +item.interval,
        interval_unit:
          item.trigger_action === DaysOfType.DaysOfDay
            ? DataMap.Interval_Unit.day.value
            : item.trigger_action === DaysOfType.DaysOfHour
            ? DataMap.Interval_Unit.hour.value
            : DataMap.Interval_Unit.minute.value,
        start_time: this.datePipe.transform(
          item.start_time,
          'yyyy-MM-dd HH:mm:ss'
        )
      }
    };

    if (DataMap.Interval_Unit.persistent.value === item.duration_unit) {
      delete params.retention.retention_duration;
      delete params.retention.duration_unit;
    }
    if (this.isCyber) {
      delete params.retention.worm_retention_duration;
      delete params.retention.worm_duration_unit;
      delete params.worm_validity_type;
    }
    return params;
  }

  getSpecifiedTimeParams(item, params) {
    if (
      includes(
        [DaysOfType.DaysOfYear, DaysOfType.DaysOfMonth, DaysOfType.DaysOfWeek],
        item.trigger_action
      )
    ) {
      delete params.schedule.start_time;
      delete params.schedule.interval;
      delete params.schedule.interval_unit;
      delete params.schedule.days_of_year;
      delete params.schedule.days_of_month;
      delete params.schedule.days_of_week;

      if (item.trigger_action === DaysOfType.DaysOfYear) {
        params.schedule.days_of_year = this.datePipe.transform(
          item.days_of_year,
          'yyyy-MM-dd'
        );
      } else if (item.trigger_action === DaysOfType.DaysOfMonth) {
        if (
          item.days_of_month_type ===
          DataMap.Days_Of_Month_Type.specifiedDate.value
        ) {
          params.schedule.days_of_month = item.days_of_month;
        } else {
          params.schedule.days_of_month = item.days_of_month_type;
        }
      } else {
        params.schedule.days_of_week = item.days_of_week;
      }

      params.retention.retention_duration = +item.specified_retention_duration;
      params.retention.retention_type =
        item.specified_duration_unit === DataMap.Interval_Unit.persistent.value
          ? RetentionType.PERMANENTLY_RETAINED
          : RetentionType.TEMPORARY_RESERVATION;
      params.retention.duration_unit = item.specified_duration_unit;
      params.retention.worm_retention_duration = item.worm_switch
        ? item.worm_validity_type === 1
          ? +item.specified_retention_duration
          : item.worm_specified_retention_duration
        : null;
      params.retention.worm_duration_unit = item.worm_switch
        ? item.worm_validity_type === 1
          ? item.specified_duration_unit
          : item.worm_specified_duration_unit
        : null;
      params.schedule.window_start = this.datePipe.transform(
        item.specified_window_start,
        'HH:mm:ss'
      );
      params.schedule.window_end = this.datePipe.transform(
        item.specified_window_end,
        'HH:mm:ss'
      );
      params.schedule.trigger_action = item.trigger_action;
    }
  }

  getExtParameters(item) {
    const params = {};
    switch (this.applicationData) {
      // 通用SLA
      case ApplicationType.Common:
        assign(params, this.getGeneralExtParameters());
        break;
      case ApplicationType.Fileset:

      case ApplicationType.NASShare:
      case ApplicationType.NASFileSystem:
      case ApplicationType.ObjectStorage:
        assign(
          params,
          this.getApplicationExtParameters(['deduplication', 'auto_index'])
        );
        break;
      case ApplicationType.Volume:
      case ApplicationType.Ndmp:
        assign(params, this.getApplicationExtParameters(['auto_index']));
        break;
      case ApplicationType.Oracle:
        assign(
          params,
          this.getApplicationExtParameters([
            'channel_number',
            'encryption',
            'encryption_algorithm',
            'deduplication',
            'compression'
          ])
        );
        break;
      case ApplicationType.Vmware:
        assign(params, this.getVmwareExtParameters());
        break;
      // 基础高级参数应用
      case ApplicationType.GaussDBDWS:
      case ApplicationType.Elasticsearch:
      case ApplicationType.Redis:
      case ApplicationType.ClickHouse:
      case ApplicationType.KubernetesDatasetCommon:
      case ApplicationType.Informix:
      case ApplicationType.MongoDB:
      case ApplicationType.GeneralDatabase:
      case ApplicationType.OceanBase:
      case ApplicationType.ActiveDirectory:
      case ApplicationType.SapHana:
        assign(params, this.getApplicationExtParameters([]));
        break;
      case ApplicationType.TiDB:
        assign(params, this.getApplicationExtParameters(['rate_limit']));
        break;
      case ApplicationType.PostgreSQL:
      case ApplicationType.AntDB:
        assign(params, this.getApplicationExtParameters(['thread_number']));
        break;
      case ApplicationType.LightCloudGaussDB:
        assign(
          params,
          this.getApplicationExtParameters([
            'rate_limit',
            'enable_standby_backup',
            'close_compression',
            'agents',
            'restart_archive'
          ])
        );
        break;
      case ApplicationType.GaussDBForOpenGauss:
        assign(
          params,
          this.getApplicationExtParameters([
            'rate_limit',
            'enable_standby_backup',
            'close_compression'
          ])
        );
        break;
      case ApplicationType.KingBase:
        assign(params, this.getApplicationExtParameters(['parallel_process']));
        break;
      case ApplicationType.MySQL:
      case ApplicationType.OpenGauss:
        assign(
          params,
          this.getApplicationExtParameters([
            'channel_number',
            'slave_node_first'
          ])
        );
        break;
      case ApplicationType.TDSQL:
        assign(
          params,
          this.getApplicationExtParameters(['channel_number', 'use_memory'])
        );
        break;
      case ApplicationType.Dameng:
      case ApplicationType.SQLServer:
      case ApplicationType.Saponoracle:
        assign(params, this.getApplicationExtParameters(['channel_number']));
        break;
      case ApplicationType.DB2:
        assign(
          params,
          this.getApplicationExtParameters([
            'copy_verify',
            'delete_log',
            'channel_number'
          ])
        );
        break;
      case ApplicationType.GoldenDB:
        assign(params, this.getApplicationExtParameters(['slave_node_first']));
        break;
      case ApplicationType.GaussDBT:
        assign(
          params,
          this.getApplicationExtParameters([
            'slave_node_first',
            'autoFullBackup',
            'parallel_process'
          ])
        );
        break;
      // 普通云平台虚拟化
      case ApplicationType.HCSCloudHost:
      case ApplicationType.FusionCompute:
      case ApplicationType.FusionOne:
      case ApplicationType.CNware:
      case ApplicationType.OpenStack:
        assign(
          params,
          this.getApplicationExtParameters([
            'copy_verify',
            'fine_grained_restore',
            'available_capacity_threshold'
          ])
        );
        break;
      case ApplicationType.ApsaraStack:
      case ApplicationType.Nutanix:
        assign(
          params,
          this.getApplicationExtParameters([
            'copy_verify',
            'fine_grained_restore'
          ])
        );
        break;
      case ApplicationType.Exchange:
      case ApplicationType.Hive:
        assign(
          params,
          this.getApplicationExtParameters(['available_capacity_threshold'])
        );
        break;
      case ApplicationType.KubernetesStatefulSet:
        assign(
          params,
          this.getApplicationExtParameters([
            'copy_verify',
            'available_capacity_threshold'
          ])
        );
        break;
      case ApplicationType.CommonShare:
        assign(params, this.getApplicationExtParameters(['deduplication']));
        break;
      case ApplicationType.HDFS:
        assign(
          params,
          this.getApplicationExtParameters([
            'auto_index',
            'concurrent_number',
            'available_capacity_threshold'
          ])
        );
        break;
      case ApplicationType.HBase:
        assign(params, this.getHBaseExtParameters(item));
        break;
      case ApplicationType.LocalFileSystem:
      case ApplicationType.LocalLun:
        assign(params, this.getLocalFileSystemExtParameters());
        break;
      case ApplicationType.HyperV:
        assign(
          params,
          this.getApplicationExtParameters([
            'copy_verify',
            'fine_grained_restore',
            'available_capacity_threshold'
          ])
        );
        break;
      default:
        assign(params, { ext_parameters: {} });
        break;
    }
    return params;
  }

  getApplicationExtParameters(params?) {
    let commonParams = [
      'qos_id',
      'auto_retry',
      'auto_retry_times',
      'auto_retry_wait_minutes',
      'alarm_over_time_window',
      'alarm_after_failure',
      'source_deduplication'
    ];

    // 去除源端重删
    if (
      [
        ApplicationType.NASFileSystem,
        ApplicationType.NASShare,
        ApplicationType.KubernetesDatasetCommon,
        ApplicationType.CommonShare,
        ApplicationType.ActiveDirectory
      ].includes(this.applicationData)
    ) {
      commonParams.pop();
    }

    // 去除任务失败告警

    let paramsList = [...commonParams, ...params];

    const ext_parameters: any = pick(this.formGroup.value, paramsList);

    // 未知的一个参数处理
    if (
      [
        ApplicationType.KingBase,
        ApplicationType.MySQL,
        ApplicationType.PostgreSQL,
        ApplicationType.AntDB,
        ApplicationType.TDSQL,
        ApplicationType.Dameng,
        ApplicationType.OpenGauss,
        ApplicationType.Informix,
        ApplicationType.MongoDB,
        ApplicationType.OceanBase,
        ApplicationType.TiDB
      ].includes(this.applicationData)
    ) {
      if (!ext_parameters.delete_policy) {
        delete ext_parameters.delete_num;
      } else {
        ext_parameters.delete_num = +ext_parameters.delete_num;
      }
    }

    // 三个云平台的qosid处理
    if (
      [
        ApplicationType.HCSCloudHost,
        ApplicationType.OpenStack,
        ApplicationType.ApsaraStack
      ].includes(this.applicationData)
    ) {
      if (!ext_parameters.qos_id) {
        delete ext_parameters.qos_id;
      }
    }

    // 部分应用qos处理
    if (
      [
        ApplicationType.Hive,
        ApplicationType.Elasticsearch,
        ApplicationType.ClickHouse,
        ApplicationType.KubernetesDatasetCommon
      ].includes(this.applicationData)
    ) {
      ext_parameters.qos_id = ext_parameters.qos_id || '';
    }

    // oracle传输和存储加密
    if (!ext_parameters.encryption) {
      delete ext_parameters.encryption_algorithm;
    }

    // oracle通道数和其他应用并发数
    if (!ext_parameters.channel_number) {
      delete ext_parameters.channel_number;
    } else {
      ext_parameters.channel_number = +ext_parameters.channel_number;
    }

    // GaussDBT/KingBase 并发进程数
    if (!ext_parameters.parallel_process) {
      delete ext_parameters.parallel_process;
    } else {
      ext_parameters.parallel_process = +ext_parameters.parallel_process;
    }

    // PostGre 线程数
    if (!ext_parameters.thread_number) {
      delete ext_parameters.thread_number;
    } else {
      ext_parameters.thread_number = +ext_parameters.thread_number;
    }

    // HDFS并发数
    if (!ext_parameters.concurrent_number) {
      delete ext_parameters.concurrent_number;
    } else {
      ext_parameters.concurrent_number = +ext_parameters.concurrent_number;
    }

    // gaussdb的流控
    if (!ext_parameters.rate_limit) {
      delete ext_parameters.rate_limit;
    } else {
      ext_parameters.rate_limit = +ext_parameters.rate_limit;
    }

    // 代理主机
    if (!isUndefined(ext_parameters.agents) && isEmpty(ext_parameters)) {
      ext_parameters.agents = [];
    }

    // 生产存储剩余容量阈值处理
    if (ext_parameters.available_capacity_threshold) {
      ext_parameters.available_capacity_threshold = Number(
        ext_parameters.available_capacity_threshold
      );
    }

    if (!ext_parameters.auto_retry) {
      delete ext_parameters.auto_retry_times;
      delete ext_parameters.auto_retry_wait_minutes;
    } else {
      ext_parameters.auto_retry_times = +ext_parameters.auto_retry_times;
      ext_parameters.auto_retry_wait_minutes = +ext_parameters.auto_retry_wait_minutes;
    }

    if (this.formGroup.value.storage_id) {
      ext_parameters.storage_info = {
        storage_type: this.formGroup.value.storage_type,
        storage_id: this.formGroup.value.storage_id
      };
    } else {
      ext_parameters.storage_info = {};
    }

    if (this.appUtilsService.isDistributed) {
      ext_parameters.enable_deduption_compression = this.formGroup.value.enable_deduption_compression;
    }
    return { ext_parameters };
  }

  getGeneralExtParameters() {
    const ext_parameters: any = pick(this.formGroup.value, [
      'alarm_over_time_window',
      'alarm_after_failure',
      'auto_retry',
      'auto_retry_times',
      'auto_retry_wait_minutes'
    ]);

    if (!ext_parameters.auto_retry) {
      delete ext_parameters.auto_retry_times;
      delete ext_parameters.auto_retry_wait_minutes;
    } else {
      ext_parameters.auto_retry_times = +ext_parameters.auto_retry_times;
      ext_parameters.auto_retry_wait_minutes = +ext_parameters.auto_retry_wait_minutes;
    }

    if (this.formGroup.value.storage_id) {
      ext_parameters.storage_info = {
        storage_type: this.formGroup.value.storage_type,
        storage_id: this.formGroup.value.storage_id
      };
    } else {
      ext_parameters.storage_info = {};
    }
    if (this.appUtilsService.isDistributed) {
      ext_parameters.enable_deduption_compression = this.formGroup.value.enable_deduption_compression;
    }
    return { ext_parameters };
  }

  getVmwareExtParameters() {
    const ext_parameters: any = pick(this.formGroup.value, [
      'fine_grained_restore',
      'ensure_consistency_backup',
      'alarm_over_time_window',
      'alarm_after_failure',
      'source_deduplication',
      'ensure_storage_layer_backup',
      'deduplication',
      'compression',
      'qos_id',
      'ensure_deleted_data',
      'ensure_specifies_transfer_mode',
      'specifies_transfer_mode',
      'auto_retry',
      'available_capacity_threshold',
      'auto_retry_times',
      'auto_retry_wait_minutes'
    ]);

    ext_parameters.available_capacity_threshold = Number(
      ext_parameters.available_capacity_threshold
    );

    if (!ext_parameters.ensure_specifies_transfer_mode) {
      ext_parameters.specifies_transfer_mode = '';
    }

    if (!ext_parameters.auto_retry) {
      delete ext_parameters.auto_retry_times;
      delete ext_parameters.auto_retry_wait_minutes;
    } else {
      ext_parameters.auto_retry_times = +ext_parameters.auto_retry_times;
      ext_parameters.auto_retry_wait_minutes = +ext_parameters.auto_retry_wait_minutes;
    }

    if (this.formGroup.value.storage_id) {
      ext_parameters.storage_info = {
        storage_type: this.formGroup.value.storage_type,
        storage_id: this.formGroup.value.storage_id
      };
    } else {
      ext_parameters.storage_info = {};
    }
    if (this.appUtilsService.isDistributed) {
      ext_parameters.enable_deduption_compression = this.formGroup.value.enable_deduption_compression;
    }
    return { ext_parameters };
  }

  getHBaseExtParameters(item) {
    const ext_parameters: any = pick(this.formGroup.value, [
      'qos_id',
      'alarm_over_time_window',
      'alarm_after_failure',
      'source_deduplication',
      'auto_retry',
      'auto_retry_times',
      'auto_retry_wait_minutes',
      'available_capacity_threshold'
    ]);

    // 生产存储剩余容量阈值处理
    if (ext_parameters.available_capacity_threshold) {
      ext_parameters.available_capacity_threshold = Number(
        ext_parameters.available_capacity_threshold
      );
    }

    if (includes([PolicyAction.FULL, PolicyAction.INCREMENT], item.action)) {
      ext_parameters['is_reserved_latest_snapshot'] =
        item.is_reserved_latest_snapshot;
      ext_parameters['backup_type'] = DataMap.Hbase_Backup_Type.snapshot.value;
    } else if (item.action === PolicyAction.SNAPSHOT) {
      delete ext_parameters['is_reserved_latest_snapshot'];
      ext_parameters['backup_type'] = DataMap.Hbase_Backup_Type.wal.value;
    } else {
      delete ext_parameters['backup_type'];
      delete ext_parameters['is_reserved_latest_snapshot'];
    }

    if (!ext_parameters.auto_retry) {
      delete ext_parameters.auto_retry_times;
      delete ext_parameters.auto_retry_wait_minutes;
    } else {
      ext_parameters.auto_retry_times = +ext_parameters.auto_retry_times;
      ext_parameters.auto_retry_wait_minutes = +ext_parameters.auto_retry_wait_minutes;
    }
    ext_parameters.qos_id = ext_parameters.qos_id || '';

    if (this.formGroup.value.storage_id) {
      ext_parameters.storage_info = {
        storage_type: this.formGroup.value.storage_type,
        storage_id: this.formGroup.value.storage_id
      };
    } else {
      ext_parameters.storage_info = {};
    }
    if (this.appUtilsService.isDistributed) {
      ext_parameters.enable_deduption_compression = this.formGroup.value.enable_deduption_compression;
    }
    return { ext_parameters };
  }

  getLocalFileSystemExtParameters() {
    const ext_parameters: any = pick(this.formGroup.value, [
      'qos_id',
      'auto_index',
      'network_acceleration',
      'open_aggregation',
      'is_synthetic_full_copy_period',
      'synthetic_full_copy_period',
      'auto_retry',
      'auto_retry_times',
      'auto_retry_wait_minutes'
    ]);

    if (!ext_parameters.auto_retry) {
      delete ext_parameters.auto_retry_times;
      delete ext_parameters.auto_retry_wait_minutes;
    } else {
      ext_parameters.auto_retry_times = +ext_parameters.auto_retry_times;
      ext_parameters.auto_retry_wait_minutes = +ext_parameters.auto_retry_wait_minutes;
    }

    if (!ext_parameters.is_synthetic_full_copy_period) {
      delete ext_parameters.synthetic_full_copy_period;
    } else {
      ext_parameters.synthetic_full_copy_period = +ext_parameters.synthetic_full_copy_period;
    }

    ext_parameters.qos_id = ext_parameters.qos_id || '';
    ext_parameters.storage_id = this.formGroup.value.backupTeams[0].storage_id;
    if (
      this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
    ) {
      assign(ext_parameters, {
        is_security_snap: this.formGroup.value.is_security_snap
      });
      delete ext_parameters.qos_id;
      delete ext_parameters.storage_id;
    }

    return { ext_parameters };
  }
  storageTypeWormChange(e) {
    this.isBasicDisk = e;
  }
  onOK(): Observable<any> {
    if (includes([ApplicationType.Dameng], this.applicationData)) {
      const fullPolicy = find(
        this.getBackupParams(),
        item => item.action === PolicyAction.FULL
      );
      const policyArray = this.getBackupParams();

      if (!fullPolicy) {
        const policyAction = map(policyArray, item => item.action);
        const errorTips = [];
        if (includes(policyAction, PolicyAction.INCREMENT)) {
          errorTips.push(this.i18n.get('protection_incremental_label'));
        }
        if (includes(policyAction, PolicyAction.LOG)) {
          errorTips.push(this.i18n.get('common_log_label'));
        }
        if (includes(policyAction, PolicyAction.DIFFERENCE)) {
          errorTips.push(this.i18n.get('common_diff_label'));
        }
        if (includes(policyAction, PolicyAction.PERMANENT)) {
          errorTips.push(this.i18n.get('protection_incremental_forever_label'));
        }
        this.messageService.error(
          this.i18n.get('protection_policy_verification_error_label', [
            errorTips.join(',')
          ])
        );
        return;
      }
    }
    const ext_parameter = this.getBackupParams()[0].ext_parameters;
    if (!!this.replicationData?.policyList.length) {
      const policyList = this.replicationData.policyList;
      if (
        find(
          policyList,
          val =>
            val?.replication_storage_id &&
            val?.replication_storage_id ===
              ext_parameter.storage_info?.storage_id
        )
      ) {
        this.messageService.error(
          this.i18n.get('protection_sla_storage_unit_repeat_label')
        );
        return;
      }
    }
    if (
      !size(
        filter(this.getBackupParams(), item => item.action !== PolicyAction.LOG)
      )
    ) {
      this.messageService.error(
        this.i18n.get('protection_policy_only_log_error_label')
      );
      return;
    }
    return new Observable<any>((observer: Observer<any>) => {
      if (this.specialResource) {
        const policyArray = this.getBackupParams();
        if (
          this.applicationData === ApplicationType.Dameng &&
          find(policyArray, item => item.action === PolicyAction.LOG)
        ) {
          this.messageService.error(
            this.i18n.get('protection_dameng_sla_vaild_label'),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'filesetSLAMessageKey'
            }
          );
          observer.error(false);
          return;
        }
        // openGauss
        if (
          this.hasOpenGauss.includes(
            DataMap.Resource_Type.OpenGauss_database.value
          ) &&
          find(policyArray, item =>
            [PolicyAction.INCREMENT, PolicyAction.LOG].includes(item.action)
          )
        ) {
          this.messageService.error(
            this.i18n.get('protection_fileset_sla_vaild_label', [
              this.i18n.get('resource_sub_type_open_gauss_database_label'),
              `${this.i18n.get(
                'common_incremental_backup_label'
              )}、${this.i18n.get('common_log_backup_label')}`
            ]),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'filesetSLAMessageKey'
            }
          );
          observer.error(false);
          return;
        }

        if (
          this.hasOpenGauss.includes(
            DataMap.Resource_Type.OpenGauss_instance.value
          ) &&
          find(policyArray, item => [PolicyAction.LOG].includes(item.action))
        ) {
          this.messageService.error(
            this.i18n.get('protection_fileset_sla_vaild_label', [
              this.i18n.get('resource_sub_type_open_gauss_instance_label'),
              this.i18n.get('common_log_backup_label')
            ]),
            {
              lvShowCloseButton: true,
              lvMessageKey: 'filesetSLAMessageKey'
            }
          );
          observer.error(false);
          return;
        }
      }
      if (this.formGroup.invalid) {
        return;
      }
      if (!this.validFormArray()) {
        observer.error(false);
        observer.complete();
        return;
      }
      const params = {
        newData: this.getBackupParams(),
        originalData: map(this.formGroup.value.backupTeams, res => {
          res.ext_parameters = omit(this.formGroup.value, [
            'backupTeams',
            'title'
          ]);
          return res;
        })
      };
      observer.next(params);
      observer.complete();
    });
  }
}
