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
import { Injectable } from '@angular/core';
import {
  BIG_DATA_ICONS,
  ClustersApiService,
  CookieService,
  COPIES_ICONS,
  DataMap,
  DaysOfType,
  defaultWindowTime,
  PolicyAction,
  QosService,
  StoragesApiService,
  STORAGE_ICONS
} from 'app/shared';
import {
  assign,
  each,
  filter,
  find,
  forEach,
  has,
  includes,
  isEmpty,
  map,
  union
} from 'lodash';
import { combineLatest, Observable, Observer } from 'rxjs';
import {
  ApplicationType,
  ApplicationTypeView,
  APP_HOST_ICONS,
  PolicyType,
  RetentionType,
  ScheduleTrigger,
  VM_ICONS
} from '..';
import { CLOUD_ICONS, CommonConsts, ReplicationModeType } from '../consts';

@Injectable({
  providedIn: 'root'
})
export class SlaParseService {
  qosItems = [];
  storageItems = [];
  externalSystemItems = [];
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isDmeUser = this.cookieService.get('userType') === CommonConsts.DME_USER_TYPE;

  constructor(
    private qosServiceApi: QosService,
    private cookieService: CookieService,
    private storageApiService: StoragesApiService,
    private clusterApiService: ClustersApiService
  ) {}

  getServices(application): Observable<void> {
    return new Observable<any>((observer: Observer<void>) => {
      const qosSubscribe = this.qosServiceApi.queryResourcesV1QosGet({
        pageNo: 0,
        pageSize: 100
      });

      if (
        includes(
          [
            ApplicationType.Fileset,
            ApplicationType.Oracle,
            ApplicationType.Vmware,
            ApplicationType.Replica
          ],
          application
        )
      ) {
        combineLatest(
          qosSubscribe,
          this.storageApiService.storageUsingGET({
            startPage: 0,
            pageSize: 200
          }),
          this.clusterApiService.getClustersInfoUsingGET({
            startPage: 0,
            pageSize: 200,
            typeList: [DataMap.Cluster_Type.target.value]
          })
        ).subscribe(
          res => {
            this.qosItems = res[0].items;
            this.storageItems = res[1].records;
            this.externalSystemItems = res[2].records;
            observer.next();
            observer.complete();
          },
          err => {
            observer.next();
            observer.complete();
          }
        );
      } else {
        combineLatest(
          qosSubscribe,
          this.clusterApiService.getClustersInfoUsingGET({
            startPage: 0,
            pageSize: 200,
            typeList: [DataMap.Cluster_Type.target.value]
          })
        ).subscribe(
          res => {
            this.qosItems = res[0].items;
            this.externalSystemItems = res[1].records;
            observer.next();
            observer.complete();
          },
          err => {
            observer.next();
            observer.complete();
          }
        );
      }
    });
  }

  getApplication(sla) {
    const icon = find(
      union(
        APP_HOST_ICONS,
        VM_ICONS,
        BIG_DATA_ICONS,
        STORAGE_ICONS,
        CLOUD_ICONS,
        COPIES_ICONS
      ),
      item => {
        return item.id === sla.application;
      }
    );
    const checkedUrl = isEmpty(icon) ? '' : 'aui-sla-' + icon.id;
    const label = isEmpty(icon) ? '' : icon.label;
    const value = sla.application;
    const viewType =
      sla.application === ApplicationType.Common
        ? ApplicationTypeView.General
        : ApplicationTypeView.Specified;
    return {
      label,
      checkedUrl,
      value,
      viewType
    };
  }

  getBackupPolicy(sla, policy_list) {
    const backupList = filter(sla.policy_list, policy => {
      return policy.type === PolicyType.BACKUP;
    });

    const policyList = [];
    each(backupList, backup => {
      policyList.push({
        uuid: backup.uuid,
        action:
          sla.application === ApplicationType.HBase
            ? backup.ext_parameters.backup_type ===
              DataMap.Hbase_Backup_Type.wal.value
              ? PolicyAction.SNAPSHOT
              : backup.action
            : backup.action,
        name:
          sla.application === ApplicationType.HBase
            ? backup.ext_parameters.backup_type ===
              DataMap.Hbase_Backup_Type.wal.value
              ? PolicyAction.SNAPSHOT
              : backup.name
            : backup.name,
        permanentBackup: has(backup, 'permanentBackup')
          ? backup.permanentBackup
          : false,
        trigger: backup.schedule.trigger,
        start_time:
          backup.schedule.trigger === ScheduleTrigger.PERIOD_EXECUTE &&
          backup.schedule.start_time
            ? new Date(backup.schedule.start_time)
            : '',
        interval: backup.schedule.interval,
        interval_unit:
          backup.schedule.interval_unit || DataMap.Interval_Unit.hour.value,
        retention_duration:
          backup.schedule.trigger === ScheduleTrigger.PERIOD_EXECUTE
            ? backup.retention.retention_duration
            : '',
        duration_unit:
          backup.retention.retention_type === RetentionType.PERMANENTLY_RETAINED
            ? DataMap.Interval_Unit.persistent.value
            : backup.schedule.trigger === ScheduleTrigger.PERIOD_EXECUTE
            ? backup.retention.duration_unit || DataMap.Interval_Unit.day.value
            : DataMap.Interval_Unit.day.value,
        window_start:
          backup.schedule.trigger === ScheduleTrigger.PERIOD_EXECUTE
            ? backup.schedule.window_start
              ? new Date(
                  `${new Date().getFullYear()}` +
                    `/` +
                    `${new Date().getMonth() + 1}` +
                    `/` +
                    `${new Date().getDate()}` +
                    ` ` +
                    `${backup.schedule.window_start}`
                )
              : defaultWindowTime()
            : defaultWindowTime(),
        window_end:
          backup.schedule.trigger === ScheduleTrigger.PERIOD_EXECUTE
            ? backup.schedule.window_end
              ? new Date(
                  `${new Date().getFullYear()}` +
                    `/` +
                    `${new Date().getMonth() + 1}` +
                    `/` +
                    `${new Date().getDate()}` +
                    ` ` +
                    `${backup.schedule.window_end}`
                )
              : defaultWindowTime()
            : defaultWindowTime(),
        specified_retention_duration:
          backup.schedule.trigger === ScheduleTrigger.SPECIFIED_TIME
            ? backup.retention.retention_duration
            : '',
        specified_duration_unit:
          backup.retention.retention_type === RetentionType.PERMANENTLY_RETAINED
            ? DataMap.Interval_Unit.persistent.value
            : backup.schedule.trigger === ScheduleTrigger.SPECIFIED_TIME
            ? backup.retention.duration_unit
            : DataMap.Interval_Unit.day.value,
        specified_window_start:
          backup.schedule.trigger === ScheduleTrigger.SPECIFIED_TIME
            ? new Date(
                `${new Date().getFullYear()}` +
                  `/` +
                  `${new Date().getMonth() + 1}` +
                  `/` +
                  `${new Date().getDate()}` +
                  ` ` +
                  `${backup.schedule.window_start}`
              )
            : defaultWindowTime(),
        specified_window_end:
          backup.schedule.trigger === ScheduleTrigger.SPECIFIED_TIME
            ? new Date(
                `${new Date().getFullYear()}` +
                  `/` +
                  `${new Date().getMonth() + 1}` +
                  `/` +
                  `${new Date().getDate()}` +
                  ` ` +
                  `${backup.schedule.window_end}`
              )
            : defaultWindowTime(),
        trigger_action:
          backup.schedule.trigger === ScheduleTrigger.PERIOD_EXECUTE
            ? backup.schedule.interval_unit === DataMap.Interval_Unit.day.value
              ? DaysOfType.DaysOfDay
              : backup.schedule.interval_unit ===
                DataMap.Interval_Unit.hour.value
              ? DaysOfType.DaysOfHour
              : DaysOfType.DaysOfMinute
            : backup.schedule.trigger_action || DaysOfType.DaysOfDay,
        days_of_year:
          backup.schedule.trigger_action === DaysOfType.DaysOfYear &&
          backup.schedule.days_of_year
            ? new Date(backup.schedule.days_of_year)
            : '',
        days_of_month_type:
          backup.schedule.days_of_month ===
            DataMap.Days_Of_Month_Type.lastDay.value &&
          backup.schedule.trigger_action === DaysOfType.DaysOfMonth
            ? DataMap.Days_Of_Month_Type.lastDay.value
            : DataMap.Days_Of_Month_Type.specifiedDate.value,
        days_of_month:
          backup.schedule.days_of_month ===
            DataMap.Days_Of_Month_Type.lastDay.value &&
          backup.schedule.trigger_action === DaysOfType.DaysOfMonth
            ? ''
            : backup.schedule.days_of_month,
        days_of_months:
          backup.schedule.days_of_month !==
            DataMap.Days_Of_Month_Type.lastDay.value &&
          backup.schedule.trigger_action === DaysOfType.DaysOfMonth
            ? map(backup.schedule.days_of_month.split(','), v => +v)
            : [],
        days_of_week:
          backup.schedule.trigger_action === DaysOfType.DaysOfWeek &&
          backup.schedule.days_of_week
            ? backup.schedule.days_of_week || []
            : [],
        is_reserved_latest_snapshot:
          backup.ext_parameters?.is_reserved_latest_snapshot,
        storage_id: backup.ext_parameters?.storage_id,
        ext_parameters: {
          ...backup.ext_parameters,
          storage_id: backup.ext_parameters?.storage_info?.storage_id,
          storage_type: backup.ext_parameters?.storage_info?.storage_type
        }
      });
      policy_list.push(backup);
    });

    return { policyList, newData: policyList };
  }

  getArchival(sla, policy_list) {
    const archivalList = filter(sla.policy_list, policy => {
      return policy.type === PolicyType.ARCHIVING;
    });

    const policyList = [];
    forEach(archivalList, archival => {
      if (
        archival.ext_parameters.archive_target_type ===
        DataMap.Archive_Target_Type.specifiedDate.value
      ) {
        const copyTypeYear = find(archival.ext_parameters.specified_scope, {
          copy_type: 'year'
        });
        if (copyTypeYear) {
          assign(archival, {
            copy_type_year: true,
            generate_time_range_year: copyTypeYear.generate_time_range,
            retention_duration_year: copyTypeYear.retention_duration
          });
        }

        const copyTypeMonth = find(archival.ext_parameters.specified_scope, {
          copy_type: 'month'
        });
        if (copyTypeMonth) {
          assign(archival, {
            copy_type_month: true,
            generate_time_range_month: copyTypeMonth.generate_time_range,
            retention_duration_month: copyTypeMonth.retention_duration
          });
        }

        const copyTypeWeek = find(archival.ext_parameters.specified_scope, {
          copy_type: 'week'
        });
        if (copyTypeWeek) {
          assign(archival, {
            copy_type_week: true,
            generate_time_range_week: copyTypeWeek.generate_time_range,
            retention_duration_week: copyTypeWeek.retention_duration
          });
        }
      }
      const qos = find(this.qosItems, {
        uuid: archival.ext_parameters.qos_id
      });
      const storage = find(this.storageItems, {
        repositoryId: archival.ext_parameters.storage_id
      });
      policyList.push({
        uuid: archival.uuid,
        name: archival.name,
        trigger: archival.schedule.trigger,
        qos_id: archival.ext_parameters.qos_id,
        protocol: archival.ext_parameters.protocol,
        driverCount: archival.ext_parameters.driverCount,
        auto_index: archival.ext_parameters.auto_index,
        qos_name: qos ? qos.name : '',
        storage_id: archival.ext_parameters.storage_id,
        storage_list: archival?.ext_parameters?.storage_list,
        storage_name: storage ? storage.name : '',
        interval: archival.schedule.interval,
        interval_unit:
          archival.schedule.interval_unit || DataMap.Interval_Unit.hour.value,
        backup_generation: archival.schedule.interval,
        retention_duration: archival.retention.retention_duration,
        duration_unit:
          archival.retention.retention_type ===
          RetentionType.PERMANENTLY_RETAINED
            ? DataMap.Interval_Unit.persistent.value
            : archival.retention.duration_unit ||
              DataMap.Interval_Unit.day.value,
        start_time: new Date(archival.schedule.start_time),
        archiving_scope:
          archival.ext_parameters.archiving_scope ||
          DataMap.Archive_Scope.latest.value,
        network_access: archival.ext_parameters.network_access,
        alarm_after_failure: archival.ext_parameters.alarm_after_failure,
        auto_retry: archival.ext_parameters.auto_retry,
        auto_retry_times: archival.ext_parameters.auto_retry_times || 3,
        auto_retry_wait_minutes:
          archival.ext_parameters.auto_retry_wait_minutes || 5,
        delete_import_copy: archival.ext_parameters.delete_import_copy,
        archive_target_type: archival.ext_parameters.archive_target_type,
        copy_type_year: archival.copy_type_year,
        generate_time_range_year:
          archival.generate_time_range_year ||
          DataMap.Year_Time_Range.December.value,
        retention_duration_year: archival.retention_duration_year,
        copy_type_month: archival.copy_type_month,
        generate_time_range_month:
          archival.generate_time_range_month ||
          DataMap.Month_Time_Range.first.value,
        retention_duration_month: archival.retention_duration_month,
        copy_type_week: archival.copy_type_week,
        generate_time_range_week:
          archival.generate_time_range_week || DataMap.Days_Of_Week.mon.value,
        retention_duration_week: archival.retention_duration_week
      });

      policy_list.push(archival);
    });

    return { policyList };
  }

  getReplication(sla, policy_list) {
    const replicationList = filter(sla.policy_list, policy => {
      return policy.type === PolicyType.REPLICATION;
    });

    const policyList = [];
    forEach(replicationList, replication => {
      const qos = find(this.qosItems, {
        uuid: replication.ext_parameters.qos_id
      });
      const storage = find(this.externalSystemItems, {
        clusterId: +replication.ext_parameters.external_system_id
      });
      if (
        replication.ext_parameters.replication_target_type ===
        DataMap.slaReplicationRule.specify.value
      ) {
        const copyTypeYear = find(replication.ext_parameters.specified_scope, {
          copy_type: 'year'
        });
        if (copyTypeYear) {
          assign(replication, {
            copy_type_year: true,
            generate_time_range_year: copyTypeYear.generate_time_range,
            retention_duration_year: copyTypeYear.retention_duration
          });
        }

        const copyTypeMonth = find(replication.ext_parameters.specified_scope, {
          copy_type: 'month'
        });
        if (copyTypeMonth) {
          assign(replication, {
            copy_type_month: true,
            generate_time_range_month: copyTypeMonth.generate_time_range,
            retention_duration_month: copyTypeMonth.retention_duration
          });
        }

        const copyTypeWeek = find(replication.ext_parameters.specified_scope, {
          copy_type: 'week'
        });
        if (copyTypeWeek) {
          assign(replication, {
            copy_type_week: true,
            generate_time_range_week: copyTypeWeek.generate_time_range,
            retention_duration_week: copyTypeWeek.retention_duration
          });
        }
      } else {
        // 复制所有副本默认年月周勾选
        assign(replication, {
          copy_type_year: true,
          copy_type_month: true,
          copy_type_week: true
        });
      }
      const reParams = {
        uuid: replication.uuid,
        name: replication.name,
        qos_id: replication.ext_parameters.qos_id,
        qos_name: qos ? qos.name : '',
        link_deduplication: replication.ext_parameters.link_deduplication,
        link_compression: replication.ext_parameters.link_compression,
        is_worm: replication.ext_parameters.is_worm ?? false,
        alarm_after_failure: replication.ext_parameters.alarm_after_failure,
        local_storage_type:
          replication.ext_parameters?.local_storage_type ||
          DataMap.poolStorageDeviceType.OceanProtectX.value,
        remote_storage_type:
          replication.ext_parameters?.remote_storage_type ||
          DataMap.poolStorageDeviceType.OceanProtectX.value,
        replication_storage_type:
          replication.ext_parameters?.replication_storage_type,
        replication_storage_id:
          replication.ext_parameters?.replication_storage_id,
        external_system_id: replication.ext_parameters.external_system_id,
        external_system_name: storage ? storage.clusterName : '',
        external_storage_id: replication?.ext_parameters?.external_storage_id,
        storage_id: replication?.ext_parameters?.storage_info?.storage_id,
        user_id: replication?.ext_parameters?.user_info?.user_id,
        userName: replication?.ext_parameters?.user_info?.username,
        replication_target_mode:
          replication?.ext_parameters?.replication_target_mode,
        backupExecuteTrigger:
          replication.schedule.trigger === ScheduleTrigger.BACKUP_EXECUTE,
        periodExecuteTrigger:
          replication.schedule.trigger !== ScheduleTrigger.BACKUP_EXECUTE,
        interval:
          replication.schedule.trigger === ScheduleTrigger.BACKUP_EXECUTE
            ? ''
            : replication.schedule.interval,
        interval_unit:
          replication.schedule.interval_unit ||
          DataMap.Interval_Unit.hour.value,
        start_time: new Date(replication.schedule.start_time),
        start_replicate_time: replication.ext_parameters.start_replicate_time
          ? new Date(replication.ext_parameters.start_replicate_time)
          : '',
        retention_duration: replication.retention.retention_duration,
        duration_unit:
          replication.retention.retention_type ===
          RetentionType.PERMANENTLY_RETAINED
            ? DataMap.Interval_Unit.persistent.value
            : replication.retention.duration_unit ||
              DataMap.Interval_Unit.day.value,
        replication_target_type:
          replication.ext_parameters.replication_target_type ??
          DataMap.slaReplicationRule.all.value,
        copy_type_year: replication.copy_type_year,
        generate_time_range_year:
          replication.generate_time_range_year ||
          DataMap.Year_Time_Range.December.value,
        retention_duration_year: replication.retention_duration_year,
        copy_type_month: replication.copy_type_month,
        generate_time_range_month:
          replication.generate_time_range_month ||
          DataMap.Month_Time_Range.first.value,
        retention_duration_month: replication.retention_duration_month,
        copy_type_week: replication.copy_type_week,
        generate_time_range_week:
          replication.generate_time_range_week ||
          DataMap.Days_Of_Week.mon.value,
        retention_duration_week: replication.retention_duration_week,
        storage_edit_disable: !!sla?.resource_count
      };
      if (this.isHcsUser) {
        assign(reParams, {
          region_code: replication.ext_parameters?.region_code,
          project_id: replication.ext_parameters?.project_id
        });
        if (
          replication.ext_parameters.replication_target_mode ===
          ReplicationModeType.CROSS_CLOUD
        ) {
          assign(reParams, {
            hcs_cluster_id: replication.ext_parameters?.hcs_cluster_id,
            tenant_name: replication.ext_parameters?.tenant_name,
            vdc_name: replication.ext_parameters?.vdc_name,
            cluster_ip: replication.ext_parameters?.cluster_ip
          });
        }
      }
      if (this.isDmeUser) {
        assign(reParams, {
          cluster_esn: replication?.ext_parameters?.cluster_esn,
          project_id: replication?.ext_parameters?.project_id
        });
      }
      policyList.push(reParams);

      policy_list.push(replication);
    });

    return { policyList };
  }
}
