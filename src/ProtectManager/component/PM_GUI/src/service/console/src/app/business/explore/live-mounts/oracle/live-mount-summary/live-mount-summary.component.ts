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
import { Component, Input, OnInit } from '@angular/core';
import {
  CommonConsts,
  CopiesService,
  CopyControllerService,
  DataMap,
  DataMapService,
  extendSummaryCopiesParams,
  HostService,
  I18NService,
  LANGUAGE,
  LiveMountAction,
  LiveMountPolicyApiService,
  ProtectedResourceApiService,
  ResourceService,
  RetentionPolicy,
  SchedulePolicy,
  SYSTEM_TIME
} from 'app/shared';
import {
  assign,
  each,
  first,
  forEach,
  includes,
  intersection,
  isEmpty,
  remove,
  set,
  size,
  toString
} from 'lodash';
import { combineLatest, Observable, Observer } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-live-mount-oracle-summary',
  templateUrl: './live-mount-summary.component.html',
  styleUrls: ['./live-mount-summary.component.less'],
  providers: [DatePipe]
})
export class LiveMountSummaryComponent implements OnInit {
  info = [];
  isHeigherVersion = false;
  schedulePolicy = SchedulePolicy;
  retentionPolicy = RetentionPolicy;
  dataMap = DataMap;

  spaceLabel = this.i18n.language === LANGUAGE.CN ? '' : ' ';
  executionPeriodLabel = this.i18n.get(
    'protection_execution_period_label',
    [],
    true
  );
  firstExecuteTimeLabel = this.i18n.get(
    'explore_first_execute_label',
    [],
    true
  );
  @Input() activeIndex;
  @Input() componentData;
  @Input() isTask;

  constructor(
    private datePipe: DatePipe,
    private i18n: I18NService,
    private hostApiService: HostService,
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private resourcesService: ResourceService,
    private copyControllerService: CopyControllerService,
    private liveMountPolicyApiService: LiveMountPolicyApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.getSummaryData();
  }

  getSummaryData() {
    if (this.activeIndex === 0) {
      return;
    }

    this.info = [];
    if (this.componentData.action === LiveMountAction.View) {
      if (!this.isTask) {
        this.getLiveMountData();
      } else {
        this.getTaskLiveMountData();
      }
    } else {
      this.getComponentData();
    }
  }

  getLiveMountData() {
    combineLatest(
      this.getResource(),
      this.getCopies(),
      this.getPolicies(),
      this.getHosts()
    ).subscribe(res => {
      const parameters = JSON.parse(
        this.componentData.liveMountData.parameters
      );
      assign(this.componentData, {
        selectionResource: res[0] || {},
        selectionCopy: res[1] || {},
        selectionPolicy: res[2] || {},
        selectionMount: assign(
          {
            targetHostList: [
              {
                name: this.componentData?.liveMountData?.target_resource_name,
                ip: this.componentData?.liveMountData?.target_resource_ip,
                version: ((res[3] as any) || {}).version
              }
            ],
            power_on: includes(
              this.componentData?.childResourceType,
              DataMap.Resource_Type.MySQLInstance.value
            )
              ? true
              : parameters.config?.power_on,
            bindWidthStatus:
              !isEmpty(toString(parameters.performance.min_bandwidth)) &&
              !isEmpty(toString(parameters.performance.max_bandwidth)),
            iopsStatus:
              !isEmpty(toString(parameters.performance.min_iops)) &&
              !isEmpty(toString(parameters.performance.max_iops)),
            latencyStatus: !isEmpty(toString(parameters.performance.latency))
          },
          parameters,
          parameters.performance
        )
      });

      if (
        !!size(
          intersection(this.componentData.childResourceType, [
            DataMap.Resource_Type.MySQLInstance.value,
            DataMap.Resource_Type.oracle.value
          ])
        )
      ) {
        this.getTargetResources();
      }

      this.getComponentData();
    });
  }

  getTaskLiveMountData() {
    const parameters = JSON.parse(
      this.componentData.liveMountData?.parameters || '{}'
    );
    assign(this.componentData, {
      selectionMount: assign(
        {
          targetHostList: [
            {
              name: parameters?.targetResourceName,
              ip: parameters?.targetResourceIp,
              version: parameters?.targetResourceVersion
            }
          ],
          power_on: includes(
            this.componentData?.childResourceType,
            DataMap.Resource_Type.MySQLInstance.value
          )
            ? true
            : parameters.config?.power_on,
          bindWidthStatus:
            !isEmpty(toString(parameters.performance.min_bandwidth)) &&
            !isEmpty(toString(parameters.performance.max_bandwidth)),
          iopsStatus:
            !isEmpty(toString(parameters.performance.min_iops)) &&
            !isEmpty(toString(parameters.performance.max_iops)),
          latencyStatus: !isEmpty(toString(parameters.performance.latency))
        },
        parameters,
        parameters.performance
      )
    });
    const selectionMount = this.getSelectionMountData();
    this.info.push(selectionMount);
  }

  compareVersion(currentV, originalV) {
    if (!currentV || !originalV) {
      return;
    }
    const arr1 = currentV.toString().split('.');
    const arr2 = originalV.toString().split('.');
    const minL = Math.min(arr1.length, arr2.length);
    let pos = 0;
    let diff = 0;
    let flag = false;
    while (pos < minL) {
      diff = parseInt(arr1[pos], 10) - parseInt(arr2[pos], 10);
      if (diff === 0) {
        pos++;
        continue;
      } else if (diff > 0) {
        flag = true;
        break;
      } else {
        flag = false;
        break;
      }
    }
    return flag;
  }

  getComponentData() {
    const selectionResource = {
      header: this.i18n.get('common_resource_label'),
      children: [
        {
          key: 'resource_name',
          label: this.i18n.get('common_name_label'),
          value: this.componentData.selectionResource.resource_name
        },
        {
          key: 'resource_environment_ip',
          label: this.i18n.get('common_ip_address_label'),
          value: includes(
            this.componentData.childResourceType,
            DataMap.Resource_Type.tdsqlInstance.value
          )
            ? this.componentData.selectionResource.resource_location
            : this.componentData.selectionResource.resource_environment_ip
        },
        {
          key: 'resource_environment_name',
          label: this.i18n.get('protection_host_cluster_label'),
          value: this.componentData.selectionResource.resource_environment_name
        }
      ]
    };

    const selectionCopy = {
      header: this.i18n.get('common_copy_data_label'),
      children: [
        {
          key: 'display_timestamp',
          label: this.i18n.get('common_time_stamp_label'),
          value: this.datePipe.transform(
            this.componentData.selectionCopy.display_timestamp,
            'yyyy-MM-dd HH:mm:ss'
          )
        },
        {
          key: 'status',
          label: this.i18n.get('common_status_label'),
          value: this.dataMapService.getLabel(
            'copydata_validStatus',
            this.componentData.selectionCopy.status
          )
        },
        {
          key: 'location',
          label: this.i18n.get('common_location_label'),
          value: this.componentData.selectionCopy.location
        },
        {
          key: 'generated_by',
          label: this.i18n.get('common_generated_type_label'),
          value: this.dataMapService.getLabel(
            'CopyData_generatedType',
            this.componentData.selectionCopy.generated_by
          )
        }
      ]
    };

    const selectionPolicy = {
      header: this.i18n.get('common_mount_update_policy_label'),
      children: [
        {
          key: 'name',
          label: this.i18n.get('common_name_label'),
          value: this.componentData.selectionPolicy.name
        },
        {
          key: 'copyDataSelectionPolicy',
          label: this.i18n.get('common_copy_data_label'),
          value: this.componentData.selectionPolicy.copyDataSelectionPolicy
        },
        {
          key: 'scheduleInterval',
          label: this.i18n.get('common_scheduled_label'),
          value: this.componentData.selectionPolicy.scheduleInterval
            ? this.i18n.get('common_param_comma_param_label', [
                `${this.executionPeriodLabel}${this.spaceLabel}${
                  this.componentData.selectionPolicy.scheduleInterval
                }${this.spaceLabel}${this.dataMapService.getLabel(
                  'Interval_Unit',
                  this.componentData.selectionPolicy.scheduleIntervalUnit
                )}`,
                `${this.firstExecuteTimeLabel}${
                  this.spaceLabel
                }${this.datePipe.transform(
                  this.componentData.selectionPolicy.scheduleStartTime,
                  'yyyy-MM-dd HH:mm:ss'
                )}`
              ])
            : '--'
        },
        {
          key: 'retentionValue',
          label: this.i18n.get('common_retention_label'),
          value: this.componentData.selectionPolicy.retentionValue
        },
        {
          key: 'liveMountCount',
          label: this.i18n.get('explore_account_of_object_label'),
          value: this.componentData.selectionPolicy.liveMountCount
        }
      ]
    };

    const selectionMount = this.getSelectionMountData();
    this.info.push(
      selectionResource,
      selectionCopy,
      selectionPolicy,
      selectionMount
    );

    if (
      includes(
        this.componentData.childResourceType,
        DataMap.Resource_Type.MySQLInstance.value
      )
    ) {
      return;
    }
    const properties =
      JSON.parse(
        this.componentData.selectionResource.resource_properties || '{}'
      ) || {};
    forEach(this.componentData.selectionMount.targetHostList, item => {
      this.isHeigherVersion = this.compareVersion(
        item['version'],
        properties.version
      );
      if (this.isHeigherVersion) {
        return false;
      }
    });
  }

  getSelectionMountData() {
    const selectionMount = {
      header: this.i18n.get('common_mount_options_label'),
      children: [
        {
          key: 'targetHostList',
          label: this.i18n.get('explore_mount_target_label'),
          value: this.componentData.selectionMount.targetHostList
        },
        {
          key: 'powerOn',
          label: this.i18n.get('explore_start_database_label'),
          value: this.dataMapService.getLabel(
            'Switch_Status',
            this.componentData.selectionMount.power_on
          )
        },
        {
          key: 'bindWidth',
          label: this.i18n.get('common_bindwidth_label'),
          value: ''
        },
        {
          key: 'iops',
          label: this.i18n.get('protection_nor_iops_label'),
          value: ''
        },
        {
          key: 'burstTime',
          label: this.i18n.get('explore_max_burst_label'),
          value: this.componentData.selectionMount.burst_time
        },
        {
          key: 'latency',
          label: this.i18n.get('protection_nor_latency_label'),
          value: this.dataMapService.getLabel(
            'LiveMount_Latency',
            this.componentData.selectionMount.latency
          )
        },
        {
          key: 'pre_script',
          label: this.i18n.get('explore_mount_pre_script_label'),
          value: this.componentData.selectionMount.pre_script
        },
        {
          key: 'post_script',
          label: this.i18n.get('explore_mount_success_script_label'),
          value: this.componentData.selectionMount.post_script
        },
        {
          key: 'failed_script',
          label: this.i18n.get('explore_mount_fail_script_label'),
          value: this.componentData.selectionMount.failed_script
        }
      ]
    };

    if (
      !(
        this.componentData.selectionMount.burst_iops ||
        this.componentData.selectionMount.burst_bandwidth
      )
    ) {
      remove(selectionMount.children, item => {
        return item.key === 'burstTime';
      });
    }

    if (
      includes(
        this.componentData.childResourceType,
        DataMap.Resource_Type.tdsqlInstance.value
      )
    ) {
      selectionMount.children.unshift({
        key: 'mysql_port',
        label: this.i18n.get('common_port_label'),
        value: this.componentData.selectionMount.mysql_port
      });
      remove(selectionMount.children, item => {
        return item.key === 'powerOn';
      });
    }
    return selectionMount;
  }

  getQuerySubType() {
    return includes(
      this.componentData.childResourceType,
      DataMap.Resource_Type.oracle.value
    )
      ? [
          DataMap.Resource_Type.oracle.value,
          DataMap.Resource_Type.oracleCluster.value
        ]
      : this.componentData.childResourceType;
  }

  getResource() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        resource_id: this.componentData.liveMountData.resource_id,
        resourceSubType: this.getQuerySubType()
      })
    };
    return this.copyControllerService.queryCopySummaryResourceV2(params).pipe(
      map(res => {
        each(res.records, item => extendSummaryCopiesParams(item));
        return first(res.records);
      })
    );
  }

  getCopies() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        uuid: this.componentData.liveMountData.copy_id,
        resource_sub_type: this.getQuerySubType()
      })
    };
    return this.copiesApiService.queryResourcesV1CopiesGet(params).pipe(
      map(res => {
        return first(res.items);
      })
    );
  }

  getPolicies() {
    if (!this.componentData.liveMountData.policy_id) {
      return new Observable<any>((observer: Observer<any>) => {
        observer.next({});
        observer.complete();
      });
    }
    return this.liveMountPolicyApiService.getPolicyUsingGET({
      policyId: this.componentData.liveMountData.policy_id
    });
  }

  getHosts() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        root_uuid: this.componentData.liveMountData.target_resource_id,
        type: 'Application',
        sub_type: DataMap.Resource_Type.OracleApp.value
      })
    };
    return this.resourcesService.queryResourcesV1ResourceGet(params).pipe(
      map(res => {
        return first(res.items);
      })
    );
  }

  getTargetResources() {
    if (!this.componentData?.liveMountData?.target_resource_id) {
      return;
    }

    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.componentData?.liveMountData?.target_resource_id
      })
      .subscribe(res => {
        set(this.componentData.selectionMount, 'targetHostList', [
          {
            name: res.name,
            ip: res.path,
            version: res.version
          }
        ]);
        this.info = [];
        this.getComponentData();
      });
  }
}
