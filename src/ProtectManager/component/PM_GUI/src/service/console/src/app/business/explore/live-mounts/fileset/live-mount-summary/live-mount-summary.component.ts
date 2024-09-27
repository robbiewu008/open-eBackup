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
  I18NService,
  LANGUAGE,
  LiveMountAction,
  LiveMountPolicyApiService,
  LocalStorageApiService,
  RetentionPolicy,
  SchedulePolicy
} from 'app/shared';
import {
  assign,
  each,
  first,
  get,
  includes,
  isEmpty,
  isUndefined,
  map as _map,
  remove,
  toString
} from 'lodash';
import { combineLatest, Observable, Observer } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-live-mount-fileset-summary',
  templateUrl: './live-mount-summary.component.html',
  styleUrls: ['./live-mount-summary.component.less'],
  providers: [DatePipe]
})
export class LiveMountSummaryComponent implements OnInit {
  info = [];
  isWindowsView = false;
  isWindows = false;
  isHeigherVersion = false;
  schedulePolicy = SchedulePolicy;
  retentionPolicy = RetentionPolicy;
  logicPortList = [];
  diverPath = '--';

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
    private dataMapService: DataMapService,
    private copiesApiService: CopiesService,
    private copyControllerService: CopyControllerService,
    private liveMountPolicyApiService: LiveMountPolicyApiService,
    private LocalStorageApiService: LocalStorageApiService
  ) {}

  ngOnInit() {
    if (
      !this.isTask &&
      this.activeIndex !== 0 &&
      get(JSON.parse(this.componentData.liveMountData.parameters), 'driveInfo')
    ) {
      this.diverPath = get(
        JSON.parse(this.componentData.liveMountData.parameters),
        'driveInfo'
      );
    }
    this.getPort();
    this.getSummaryData();
  }

  getShareIP() {
    this.LocalStorageApiService.getLogicPortUsingGET({
      akDoException: false,
      protocol: '2,3'
    }).subscribe(
      res => {
        this.getComponentData(res.logicPortList);
        this.logicPortList = res.logicPortList;
      },
      () => {
        this.getComponentData();
      }
    );
  }

  getPort() {
    this.LocalStorageApiService.getLogicPortUsingGET({
      akDoException: false,
      protocol: '2,3'
    }).subscribe(
      res => {
        this.logicPortList = res.logicPortList;
      },
      () => {
        this.getComponentData();
      }
    );
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
      this.getPolicies()
    ).subscribe(res => {
      const parameters = JSON.parse(
        this.componentData.liveMountData?.parameters || '{}'
      );
      assign(this.componentData, {
        selectionResource: res[0] || {},
        selectionCopy: res[1] || {},
        selectionPolicy: res[2] || {},
        selectionMount: assign(
          {
            ip: this.componentData?.liveMountData?.target_resource_ip,
            metadataPath: [
              {
                label: parameters.dstPath
              }
            ],
            power_on: parameters.config?.power_on,
            bindWidthStatus:
              !isEmpty(toString(parameters.performance?.min_bandwidth)) ||
              !isEmpty(toString(parameters.performance?.max_bandwidth)),
            iopsStatus:
              !isEmpty(toString(parameters.performance?.min_iops)) ||
              !isEmpty(toString(parameters.performance?.max_iops)),
            latencyStatus: !isEmpty(toString(parameters?.performance?.latency))
          },
          parameters,
          parameters.performance
        )
      });
      this.getShareIP();
    });
  }

  getTaskLiveMountData() {
    const parameters = JSON.parse(
      this.componentData.liveMountData?.parameters || '{}'
    );
    assign(this.componentData, {
      selectionMount: assign(
        {
          ip: parameters?.targetResourceIp,
          metadataPath: [
            {
              label: parameters.dstPath
            }
          ],
          power_on: parameters.config?.power_on,
          bindWidthStatus:
            !isEmpty(toString(parameters.performance?.min_bandwidth)) ||
            !isEmpty(toString(parameters.performance?.max_bandwidth)),
          iopsStatus:
            !isEmpty(toString(parameters.performance?.min_iops)) ||
            !isEmpty(toString(parameters.performance?.max_iops)),
          latencyStatus: !isEmpty(toString(parameters?.performance?.latency))
        },
        parameters,
        parameters.performance
      )
    });
    const selectionMount = this.getSelectionMountData([]);
    this.info.push(selectionMount);
  }

  compareVersion(currentV, originalV) {
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

  getComponentData(logicPortList?) {
    let port = this.logicPortList;
    if (isUndefined(port)) {
      port = logicPortList;
    }

    let windowsPath = '';
    if (
      this.componentData.action === LiveMountAction.View &&
      (this.isWindows ||
        JSON.parse(
          this.componentData?.selectionCopy?.resource_properties || '{}'
        )['environment_os_type'] === DataMap.Os_Type.windows.value)
    ) {
      const info = this.componentData.selectionMount?.driveInfo;
      windowsPath = info.split('&')[0];
    }

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
          value: this.componentData.selectionResource.resource_environment_ip
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
          key: 'mountPath',
          label: this.i18n.get('system_volume_path_label'),
          hidden: this.componentData.action !== LiveMountAction.View,
          value: includes(
            this.componentData.childResourceType,
            DataMap.Resource_Type.volume.value
          )
            ? `${this.componentData.selectionMount?.dstPath}/volumelivemount/${this.componentData.liveMountData?.mounted_copy_id}/volumes`
            : this.isWindows ||
              JSON.parse(
                this.componentData?.selectionCopy?.resource_properties || '{}'
              )['environment_os_type'] === DataMap.Os_Type.windows.value
            ? windowsPath
            : this.componentData.selectionMount?.dstPath
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
    const selectionMount = this.getSelectionMountData(
      !!logicPortList?.length ? logicPortList : []
    );

    if (!this.isTask) {
      this.info.push(
        selectionResource,
        selectionCopy,
        selectionPolicy,
        selectionMount
      );
    } else {
      this.info.push(selectionMount);
    }

    if (
      includes(
        this.componentData.childResourceType,
        DataMap.Resource_Type.MySQLInstance.value
      )
    ) {
      return;
    }
  }

  getSelectionMountData(logicPortList?) {
    let port = this.logicPortList;
    if (isUndefined(port)) {
      port = logicPortList;
    }
    let mountInfo = {
      shareName: '',
      type: 1,
      userName: []
    };
    if (this.activeIndex === 2) {
      assign(mountInfo, {
        shareName: this.componentData.share_name,
        type: this.componentData.type,
        userName: this.componentData.userName
      });
    } else if (isUndefined(this.activeIndex)) {
      const liveMountInfo = JSON.parse(
        this.componentData.liveMountData.file_system_share_info
      )[0];
      assign(mountInfo, {
        shareName: liveMountInfo.advanceParams?.shareName,
        type: liveMountInfo.advanceParams?.domainType,
        userName: liveMountInfo.advanceParams?.usernames
      });
    }
    if (mountInfo.type === DataMap.Cifs_Domain_Client_Type.everyone.value) {
      mountInfo.userName = ['--'];
    }
    const selectionMount = {
      header: this.i18n.get('common_mount_options_label'),
      children: [
        {
          key: 'ip',
          label: this.i18n.get('common_ip_label'),
          value: this.componentData.selectionMount?.ip
        }
      ]
    };
    if (
      !isUndefined(this.componentData.action) &&
      JSON.parse(
        this.componentData?.selectionCopy?.resource_properties || '{}'
      )['environment_os_type'] === DataMap.Os_Type.windows.value
    ) {
      selectionMount.children = selectionMount.children.concat([
        {
          key: 'devPath',
          label: this.i18n.get('common_mount_diver_letter_label'),
          value: this.diverPath
        }
      ]);
    }
    if (
      this.isWindows ||
      JSON.parse(
        this.componentData?.selectionCopy?.resource_properties || '{}'
      )['environment_os_type'] === DataMap.Os_Type.windows.value
    ) {
      this.isWindowsView =
        JSON.parse(
          this.componentData?.selectionCopy?.resource_properties || '{}'
        )['environment_os_type'] === DataMap.Os_Type.windows.value;
      selectionMount.children = selectionMount.children.concat([
        {
          key: 'cifs_shares_name',
          label: this.i18n.get('common_cifs_shares_name_label'),
          value: mountInfo.shareName
        },
        {
          key: 'type',
          label: this.i18n.get('common_type_label'),
          value: this.dataMapService.getLabel(
            'Cifs_Domain_Client_Type',
            mountInfo.type
          )
        },
        {
          key: 'users',
          label: this.i18n.get('common_users_label'),
          value: mountInfo.userName
        },
        {
          key: 'shareIp',
          label: this.i18n.get('protection_shared_ip_label'),
          value: _map(port, item => {
            return { ip: item };
          })
        }
      ]);
    }
    selectionMount.children = selectionMount.children.concat([
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
      }
    ]);

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
    return selectionMount;
  }

  getResource() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        resource_id: this.componentData.liveMountData.resource_id,
        resourceSubType: this.componentData.childResourceType
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
        resource_sub_type: this.componentData.childResourceType
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
}
