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
import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { MessageService, ModalRef } from '@iux/live';
import {
  ApplicationType,
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  PolicyType,
  ReplicationModeType,
  RetentionType,
  ScheduleTrigger
} from 'app/shared';
import { SlaValidatorService } from 'app/shared/services/sla-validator.service';
import {
  assign,
  each,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  set,
  size,
  uniqBy
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { AppUtilsService } from '../../../../../../shared/services/app-utils.service';
import { ReplicationPolicyComponent } from './replication-policy/replication-policy.component';

@Component({
  selector: 'aui-specified-replication-policy',
  templateUrl: './specified-replication-policy.component.html',
  styleUrls: ['./specified-replication-policy.component.less'],
  providers: [DatePipe]
})
export class SpecifiedReplicationPolicyComponent implements OnInit {
  action;
  activeIndex;
  backupData;
  replicationData;
  applicationData;
  formGroup: FormGroup;
  applicationType = ApplicationType;
  application;
  replicationModeType = ReplicationModeType;
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isDmeUser = this.cookieService.get('userType') === CommonConsts.DME_USER_TYPE;
  @ViewChild(ReplicationPolicyComponent, { static: false })
  ReplicationPolicyComponent: ReplicationPolicyComponent;

  constructor(
    public baseUtilService: BaseUtilService,
    public i18n: I18NService,
    public fb: FormBuilder,
    public modal: ModalRef,
    public datePipe: DatePipe,
    private cookieService: CookieService,
    public dataMapService: DataMapService,
    private messageService: MessageService,
    private slaValidatorService: SlaValidatorService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.formGroup = this.fb.group({});
    setTimeout(() => {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    }, 0);
  }

  validFormArray() {
    const errorMsgs = [];
    const replicationTeams = this.formGroup.value.replicationTeams;
    const nameItems = uniqBy(this.formGroup.value.replicationTeams, 'name');
    if (size(nameItems) !== size(replicationTeams)) {
      this.messageService.error(
        this.i18n.get('protection_external_same_repeats_label'),
        {
          lvMessageKey: 'lvMsg_sla_valid_error',
          lvShowCloseButton: true
        }
      );
      return false;
    }

    const replicationCluster = [];
    each(this.formGroup.value.replicationTeams, item => {
      if (item?.external_system_id) {
        replicationCluster.push({
          external_system_id: item.external_system_id,
          action: item.action
        });
      }
    });
    const dataClusterItems = uniqBy(
      replicationCluster.filter(
        item => item.action === DataMap.replicationAction.data.value
      ),
      'external_system_id'
    );
    const logClusterItems = uniqBy(
      replicationCluster.filter(
        item => item.action === DataMap.replicationAction.log.value
      ),
      'external_system_id'
    );
    if (
      size(dataClusterItems) + size(logClusterItems) !==
      size(replicationCluster)
    ) {
      this.messageService.error(
        this.i18n.get('protection_external_system_same_repeats_label'),
        {
          lvMessageKey: 'lvMsg_sla_valid_error',
          lvShowCloseButton: true
        }
      );
      return false;
    }

    for (const item of replicationTeams) {
      if (
        item.periodExecuteTrigger &&
        item.replication_target_type === DataMap.slaReplicationRule.all.value
      ) {
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
            this.i18n.get(invalidIntervalResult, [item.name]),
            {
              lvMessageKey: 'lvMsg_sla_valid_error',
              lvShowCloseButton: true
            }
          );
          break;
        }
      }
    }

    return !size(errorMsgs);
  }

  // NAS共享、NAS文件系统、文件集、对象存储的备份SLA目标端重删关闭，复制的SLA链路重删不能开启
  validLinkRedelete(): boolean {
    // E6000不会下发这两个参数，所以不用管
    if (this.appUtilsService.isDistributed) {
      return true;
    }
    const backpExtParams: any = isArray(this.backupData)
      ? this.backupData[0]?.ext_parameters
      : {};
    const hasLinkDeduplication = find(
      this.formGroup.value.replicationTeams,
      item => item.link_deduplication
    );
    if (
      includes(
        [
          ApplicationType.NASFileSystem,
          ApplicationType.NASShare,
          ApplicationType.Fileset,
          ApplicationType.ObjectStorage
        ],
        this.applicationData
      ) &&
      !backpExtParams?.deduplication &&
      !isEmpty(hasLinkDeduplication)
    ) {
      this.messageService.error(
        this.i18n.get('protection_link_redelete_error_tip_label'),
        {
          lvMessageKey: 'lvMsg_key_link_deduplication_error_label',
          lvShowCloseButton: true
        }
      );
      return false;
    }
    return true;
  }

  getReplicationParams() {
    const replicationTeams = [];
    each(this.formGroup.value.replicationTeams, (item, index) => {
      replicationTeams.push(this.getParams(item, index));
    });
    return replicationTeams;
  }

  getParams(item, index) {
    const _backupData = first(this.backupData);
    const trigger = get(_backupData, 'trigger');
    const params = {
      uuid: item.uuid,
      name: item.name,
      action: item.action,
      type: PolicyType.REPLICATION,
      ext_parameters: {
        specified_scope: [],
        replication_target_type: item.replication_target_type,
        local_storage_type: item?.local_storage_type,
        remote_storage_type: item?.remote_storage_type,
        qos_id: item.qos_id,
        replication_storage_type: item?.replication_storage_type,
        external_system_id: item.external_system_id,
        link_deduplication: item.link_deduplication,
        link_compression: item.link_compression,
        alarm_after_failure: item.alarm_after_failure,
        start_replicate_time: this.datePipe.transform(
          item.start_replicate_time,
          'yyyy-MM-dd HH:mm:ss'
        )
      },
      retention: {
        retention_type:
          DataMap.Interval_Unit.persistent.value === item.duration_unit
            ? RetentionType.PERMANENTLY_RETAINED
            : RetentionType.TEMPORARY_RESERVATION,
        retention_duration: +item.retention_duration,
        duration_unit: item.duration_unit
      },
      schedule: {
        trigger: item.backupExecuteTrigger
          ? ScheduleTrigger.BACKUP_EXECUTE
          : ScheduleTrigger.PERIOD_EXECUTE,
        interval: +item.interval,
        interval_unit: item.interval_unit,
        start_time: this.datePipe.transform(
          item.start_time,
          'yyyy-MM-dd HH:mm:ss'
        )
      }
    };

    if (
      item.replication_target_type === DataMap.slaReplicationRule.specify.value
    ) {
      const specified_scope = [];
      if (!params.retention.retention_duration) {
        params.retention.retention_duration = 1;
      }
      if (item.copy_type_year) {
        specified_scope.push({
          copy_type: 'year',
          generate_time_range: item.generate_time_range_year,
          retention_unit: 'y',
          retention_duration: +item.retention_duration_year
        });
      }
      if (item.copy_type_month) {
        specified_scope.push({
          copy_type: 'month',
          generate_time_range: item.generate_time_range_month,
          retention_unit: 'MO',
          retention_duration: +item.retention_duration_month
        });
      }
      if (item.copy_type_week) {
        specified_scope.push({
          copy_type: 'week',
          generate_time_range: item.generate_time_range_week,
          retention_unit: 'w',
          retention_duration: +item.retention_duration_week
        });
      }
      params.ext_parameters.specified_scope = specified_scope;
      // 删除复制所有副本字段
      delete params.ext_parameters.start_replicate_time;
    } else {
      delete params.ext_parameters.specified_scope;
    }

    if (
      this.application === DataMap.Resource_Type.GaussDB_DWS.value &&
      item.external_storage_id
    ) {
      set(
        params,
        'ext_parameters.external_storage_id',
        item.external_storage_id
      );
    }
    if (item.duration_unit === DataMap.Interval_Unit.persistent.value) {
      delete params.retention.retention_duration;
      delete params.retention.duration_unit;
    }
    if (
      this.isDataBackup ||
      this.appUtilsService.isDecouple ||
      this.appUtilsService.isDistributed
    ) {
      set(
        params,
        'ext_parameters.replication_target_mode',
        item.replicationMode
      );
      if (this.appUtilsService.isDistributed) {
        delete params.ext_parameters.link_deduplication;
      }
      if (
        item.replicationMode === this.replicationModeType.CROSS_DOMAIN &&
        (item.external_storage_id || item.replication_storage_id) &&
        !this.isDmeUser
      ) {
        set(params, 'ext_parameters.storage_info', {
          storage_id:
            item.replication_storage_type ===
            DataMap.backupStorageTypeSla.group.value
              ? item.external_storage_id
              : item.replication_storage_id,
          storage_type:
            item.replication_storage_type ===
            DataMap.backupStorageTypeSla.group.value
              ? 'storage_unit_group'
              : 'storage_unit'
        });
        set(params, 'ext_parameters.user_info', {
          user_id: item.specifyUser,
          username: item.userName,
          password: item.authPassword || undefined,
          userType: item.userType
        });
      }

      if (
        this.isDmeUser &&
        item.replicationMode === ReplicationModeType.CROSS_DOMAIN
      ) {
        delete params.ext_parameters.external_system_id;
        delete params.ext_parameters.replication_storage_type; // 指定目标位置
        assign(params.ext_parameters, {
          project_id: item.project_id,
          cluster_esn: item.cluster_esn
        });
      }

      if (
        item.replicationMode === this.replicationModeType.INTRA_DOMAIN &&
        item.external_storage_id &&
        item.replication_storage_type ===
          DataMap.backupStorageTypeSla.group.value
      ) {
        set(params, 'ext_parameters.storage_info', {
          storage_id: item.external_storage_id,
          storage_type: 'storage_unit_group'
        });
      }
      if (
        item.replicationMode === this.replicationModeType.INTRA_DOMAIN &&
        item.replication_storage_id &&
        item.replication_storage_type ===
          DataMap.backupStorageTypeSla.unit.value
      ) {
        set(params, 'ext_parameters.storage_info', {
          storage_id: item.replication_storage_id,
          storage_type: 'storage_unit'
        });
      }
      if (
        this.application === DataMap.Resource_Type.GaussDB_DWS.value &&
        item.external_storage_id
      ) {
        set(
          params,
          'ext_parameters.external_storage_id',
          item.external_storage_id
        );
      }
    }

    if (this.isHcsUser) {
      delete params.ext_parameters.external_system_id;
      delete params.ext_parameters.replication_storage_type;
      if (item.replicationMode === ReplicationModeType.INTRA_DOMAIN) {
        set(
          params,
          'ext_parameters.replication_storage_type',
          item?.replication_storage_type
        );
      } else {
        set(
          params,
          'ext_parameters.region_code',
          item.external_system_id[0]?.parent?.region_id ||
            item.external_system_id[0]?.region_id
        );
        set(
          params,
          'ext_parameters.project_id',
          item.external_system_id[0]?.id
        );
      }
      if (item.replicationMode === ReplicationModeType.CROSS_CLOUD) {
        assign(params.ext_parameters, {
          source_cluster_ip: item.external_system_id[0]?.source_cluster_ip,
          hcs_cluster_id: item.hcs_cluster_id,
          vdc_name: item.vdc_name,
          tenant_name: item.tenant_name,
          cluster_ip: item.external_system_id[0]?.cluster_ip.split(',')[0],
          replication_storage_type: item?.replication_storage_type
        });
        set(params, 'ext_parameters.storage_info', {
          storage_id:
            item.replication_storage_type ===
            DataMap.backupStorageTypeSla.group.value
              ? item.external_storage_id
              : item.replication_storage_id,
          storage_type:
            item.replication_storage_type ===
            DataMap.backupStorageTypeSla.group.value
              ? 'storage_unit_group'
              : 'storage_unit'
        });
      }
    }

    return params;
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }

      if (!this.validFormArray()) {
        observer.error(false);
        observer.complete();
        return;
      }

      if (!this.validLinkRedelete()) {
        observer.error(false);
        observer.complete();
        return;
      }

      const params = {
        newData: this.getReplicationParams(),
        originalData: this.formGroup.value.replicationTeams
      };

      observer.next(params);
      observer.complete();
    });
  }
}
