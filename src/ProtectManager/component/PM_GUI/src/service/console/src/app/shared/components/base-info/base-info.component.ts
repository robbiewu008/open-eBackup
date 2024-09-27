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
import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import {
  HostService,
  ProjectedObjectApiService
} from 'app/shared/api/services';
import { CopiesService } from 'app/shared/api/services/copies.service';
import { CommonConsts } from 'app/shared/consts';
import {
  DataMapService,
  GlobalService,
  I18NService
} from 'app/shared/services';
import { assign, find, isNil, remove } from 'lodash';
import { combineLatest, Observable, Observer, Subscription } from 'rxjs';
import { DataMap } from './../../consts/data-map.config';

@Component({
  selector: 'resoure-base-info',
  templateUrl: './base-info.component.html',
  styleUrls: ['./base-info.component.less'],
  providers: [DatePipe]
})
export class BaseInfoComponent implements OnInit, OnDestroy {
  resourceType = DataMap.Resource_Type;
  isTidbDatabase = false;
  dataMap = DataMap;
  @Input() source;
  @Input() sourceType;
  formItems: any = [
    [
      {
        key: 'name',
        label: this.i18n.get('common_name_label'),
        content: ''
      },
      {
        key: 'type',
        label: this.i18n.get('common_type_label'),
        content: ''
      },
      {
        key: 'link_status',
        label: this.i18n.get('common_status_label'),
        content: ''
      }
    ],
    [
      {
        key: 'protect_activation',
        label: this.i18n.get('protection_protected_status_label'),
        content: ''
      },
      {
        key: 'sla_policy',
        label: this.i18n.get('common_sla_label'),
        content: ''
      },
      {
        key: 'sla_compliance',
        label: this.i18n.get('common_sla_compliance_label'),
        content: ''
      }
    ],
    [
      {
        key: 'last_time',
        label: this.i18n.get('protection_last_time_label'),
        content: ''
      },
      {
        key: 'earliest_time',
        label: this.i18n.get('protection_earliest_time_label'),
        content: ''
      },
      {
        key: 'next_time',
        label: this.i18n.get('protection_next_time_label'),
        content: ''
      }
    ],
    [
      {
        key: 'copies',
        label: this.i18n.get('common_copy_count_label'),
        content: 0
      }
    ]
  ];
  subscription$: Subscription;

  constructor(
    public datePipe: DatePipe,
    public i18n: I18NService,
    public hostServiceAPI: HostService,
    public copiesApiService: CopiesService,
    public dataMapService: DataMapService,
    public projectedObjectApiService: ProjectedObjectApiService,
    private globalService: GlobalService
  ) {}

  ngOnInit() {
    if (this.sourceType === DataMap.Resource_Type.vmGroup.value) {
      this.formItems.pop();
    }
    this.getResourceInfo();
    this.getCopyInfo();
    this.getAutoRefresh();
    this.isTidbDatabase =
      this.sourceType === DataMap.Resource_Type.tidbDatabase.value;
  }

  ngOnDestroy() {
    this.subscription$.unsubscribe();
  }

  getCopies(params?): any {
    return this.copiesApiService.queryResourcesV1CopiesGet({
      pageSize: CommonConsts.PAGE_SIZE,
      pageNo: CommonConsts.PAGE_START,
      conditions: JSON.stringify(
        assign(
          {
            resource_id: this.source.uuid
          },
          params
        )
      )
    });
  }

  getAutoRefresh() {
    this.subscription$ = this.globalService
      .getState('autoReshResource')
      .subscribe(res => {
        assign(this.source, res);
        this.getResourceInfo();
      });
  }

  getAllCopies() {
    return new Observable<void>((observer: Observer<any>) => {
      this.getCopies().subscribe(
        res => {
          observer.next(res);
          observer.complete();
        },
        err => {
          observer.next({});
          observer.complete();
        }
      );
    });
  }

  getSyncTime() {
    return new Observable<void>((observer: Observer<any>) => {
      if (this.source.sla_id) {
        this.projectedObjectApiService
          .queryProtectionTimeV1ProtectedObjectsResourceIdBackupTimeGet({
            resourceId: this.source.uuid
          })
          .subscribe(
            res => {
              observer.next(res);
              observer.complete();
            },
            err => {
              observer.next({});
              observer.complete();
            }
          );
      } else {
        observer.next({});
        observer.complete();
      }
    });
  }

  getCopyInfo() {
    const combined: any = combineLatest(
      this.getAllCopies(),
      this.getSyncTime()
    );
    combined.subscribe(resArr => {
      const [all, syncTime] = resArr;
      if (this.sourceType !== DataMap.Resource_Type.vmGroup.value) {
        this.formItems[3][0].content = all.total;
      }
      this.formItems[2][0].content = syncTime.earliest_time
        ? this.datePipe.transform(
            new Date(syncTime.earliest_time),
            'yyyy-MM-dd HH:mm:ss'
          )
        : '';
      this.formItems[2][1].content = syncTime.latest_time
        ? this.datePipe.transform(
            new Date(syncTime.latest_time),
            'yyyy-MM-dd HH:mm:ss'
          )
        : '';
      this.formItems[2][2].content = !this.source.sla_status
        ? ''
        : syncTime.next_time
        ? this.datePipe.transform(
            new Date(syncTime.next_time),
            'yyyy-MM-dd HH:mm:ss'
          )
        : '';
    });
  }

  getResourceInfo() {
    switch (this.sourceType) {
      case DataMap.Resource_Type.vmGroup.value:
        this.removeType();
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        break;
      case DataMap.Resource_Type.ExchangeEmail.value: {
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.address;
        this.formItems[0][1].label = this.i18n.get('common_address_label');
        break;
      }
      case DataMap.Resource_Type.ObjectSet.value:
        this.formItems[0][0].content = this.source.name;
        this.removeLinkStatus();
        this.formItems[0][1].content = this.source.environment.name;
        this.formItems[0][1].label = this.i18n.get(
          'protection_object_storage_owned_label'
        );
        break;
      case DataMap.Resource_Type.ExchangeDataBase.value: {
        this.formItems[0][0].content = this.source.name;
        this.removeLinkStatus();
        this.formItems[0][1].content = this.source.environment.name;
        this.formItems[0][1].label = this.i18n.get(
          'protection_single_node_system_group_tag_label'
        );
        break;
      }
      case DataMap.Resource_Type.OceanBaseCluster.value: {
        this.removeType();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.linkStatus;
        break;
      }
      case DataMap.Resource_Type.OceanBaseTenant.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = `${this.source?.environment?.name} / ${this.source.name}`;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        this.formItems[0][2].content = this.source.extendInfo?.linkStatus;
        break;
      }
      case DataMap.Resource_Type.tidbDatabase.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content =
          this.source.parentName || this.source.parent_name;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        this.formItems[0][2].content =
          this.source.linkStatus === '0' ? '1' : '0';
        this.formItems[0][3] = this.formItems[0][2];
        this.formItems[0][2] = {
          key: 'name',
          label: this.i18n.get('protection_host_database_name_label'),
          content: this.source.extendInfo.databaseName
        };
        break;
      }
      case DataMap.Resource_Type.tidbTable.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content =
          this.source.parentName || this.source.parent_name;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        this.formItems[0][2].content =
          this.source.linkStatus === '0' ? '1' : '0';
        break;
      }
      case DataMap.Resource_Type.tdsqlDistributedInstance.value:
      case DataMap.Resource_Type.tdsqlInstance.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content =
          this.source?.environment?.name || this.source?.environment_name;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        this.formItems[0][2].content = this.source.extendInfo?.linkStatus;
        break;
      }
      case DataMap.Resource_Type.lightCloudGaussdbProject.value:
      case DataMap.Resource_Type.lightCloudGaussdbInstance.value:
      case DataMap.Resource_Type.ActiveDirectory.value:
        this.formItems[0][0].content = this.source.name;
        this.removeType();
        this.removeLinkStatus();
        break;
      case DataMap.Resource_Type.gaussdbForOpengaussInstance.value:
      case DataMap.Resource_Type.gaussdbForOpengaussProject.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][2].content =
          this.source.linkStatus || this.source.extendInfo?.status;
        break;
      }
      case DataMap.Resource_Type.goldendbInstance.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][2].content = this.source.linkStatus;
        break;
      }
      case DataMap.Resource_Type.informixClusterInstance.value:
      case DataMap.Resource_Type.informixInstance.value:
      case DataMap.Resource_Type.gbaseInstance.value:
      case DataMap.Resource_Type.gbaseClusterInstance.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.environment?.name;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        this.formItems[0][2].content = this.source.extendInfo?.linkStatus;
        break;
      }
      case DataMap.Resource_Type.generalDatabase.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.extendInfo?.databaseTypeDisplay;
        this.formItems[0][2].content = this.source.linkStatus;
        break;
      }
      case DataMap.Resource_Type.DWS_Cluster.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][2].content = this.source.linkStatus;
        this.removeType();
        break;
      }
      case DataMap.Resource_Type.dbTwoDatabase.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = `${this.source.clusterOrHostName}/${this.source.parentName}`;
        this.formItems[0][1].label = this.i18n.get(
          'protection_host_cluster_name_label'
        );
        this.removeLinkStatus();
        break;
      }
      case DataMap.Resource_Type.dbTwoTableSet.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = `${this.source.clusterOrHostName}/${this.source.extendInfo?.instance}/${this.source.parentName}`;
        this.formItems[0][1].label = this.i18n.get(
          'protection_host_database_name_label'
        );
        this.removeLinkStatus();
        break;
      }
      case DataMap.Resource_Type.DWS_Table.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.clusterOrHostName;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        this.removeLinkStatus();
        break;
      }
      case DataMap.Resource_Type.DWS_Schema.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.clusterOrHostName;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        this.removeLinkStatus();
        break;
      }
      case DataMap.Resource_Type.ABBackupClient.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.dataMapService.getLabel(
          'Os_Type',
          this.source.os_type
        );
        this.formItems[0][1].label = this.i18n.get('common_system_type_label');
        this.formItems[0][2].content = this.source.link_status;
        break;
      }
      case DataMap.Resource_Type.volume.value:
      case DataMap.Resource_Type.fileset.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.environment?.name;
        this.formItems[0][1].label = this.i18n.get(
          'protection_host_name_label'
        );
        this.formItems[0][2].content = this.source.environment?.endpoint;
        this.formItems[0][2].label = this.i18n.get('common_ip_address_label');
        break;
      }
      case DataMap.Resource_Type.SQLServerDatabase.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = `${this.source.clusterOrHostName}/${this.source.ownedInstance}`;
        this.formItems[0][1].label = this.i18n.get(
          'commom_owned_instance_label'
        );
        this.removeLinkStatus();
        break;
      }
      case DataMap.Resource_Type.SQLServerClusterInstance.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.clusterOrHostName;
        this.formItems[0][1].label = this.i18n.get(
          'protection_host_cluster_name_label'
        );
        this.removeLinkStatus();
        break;
      }
      case DataMap.Resource_Type.SQLServerInstance.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.clusterOrHostName;
        this.formItems[0][1].label = this.i18n.get(
          'protection_host_cluster_name_label'
        );
        this.removeLinkStatus();
        break;
      }
      case DataMap.Resource_Type.SQLServerGroup.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.clusterOrHostName;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        this.removeLinkStatus();
        break;
      }
      case DataMap.Resource_Type.MySQLClusterInstance.value:
      case DataMap.Resource_Type.MySQLInstance.value:
      case DataMap.Resource_Type.MySQLDatabase.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.environment?.name;
        this.formItems[0][2].content = this.source?.auth_status;
        this.formItems[0][1].label = this.i18n.get(
          'protection_host_cluster_name_label'
        );
        if (this.sourceType === DataMap.Resource_Type.MySQLDatabase.value) {
          this.removeLinkStatus();
        }
        break;
      }
      case DataMap.Resource_Type.oracle.value:
      case DataMap.Resource_Type.oracleCluster.value:
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.ip;
        this.formItems[0][1].label = this.i18n.get('common_ip_address_label');
        break;
      case DataMap.Resource_Type.SQLServer.value:
      case DataMap.Resource_Type.DB2.value: {
        this.removeType();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.link_status;
        break;
      }
      case DataMap.Resource_Type.gaussdbTSingle.value:
      case DataMap.Resource_Type.GaussDB_T.value: {
        this.removeType();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.clusterState;
        break;
      }
      case DataMap.Resource_Type.DWS_Database.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.environment?.name;
        this.formItems[0][1].label = this.i18n.get(
          'protection_host_cluster_name_label'
        );
        break;
      }
      case DataMap.Resource_Type.virtualMachine.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.i18n.get('common_vmware_label');
        this.formItems[0][2].content = this.source.link_status;
        break;
      }
      case DataMap.Resource_Type.msVirtualMachine.value: {
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.i18n.get('common_hyperv_label');
        break;
      }
      case DataMap.Resource_Type.ImportCopy.value: {
        this.removeType();
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        break;
      }
      case DataMap.Resource_Type.NASFileSystem.value:
      case DataMap.Resource_Type.NASShare.value:
      case DataMap.Resource_Type.ndmp.value:
        this.removeType();
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        if (
          !(
            this.sourceType === DataMap.Resource_Type.NASShare.value &&
            (this.source.environment?.subType ===
              DataMap.Device_Storage_Type.Other.value ||
              this.source.environment_sub_type ===
                DataMap.Device_Storage_Type.Other.value)
          )
        ) {
          this.formItems[0].push(
            {
              key: 'parentName',
              label: this.i18n.get('protection_storage_device_label'),
              content: this.source.parentName || this.source.parent_name
            },
            {
              key: 'parentIp',
              label: this.i18n.get('common_ip_address_label'),
              content:
                this.source.environment?.endpoint ||
                this.source.environment_endpoint
            }
          );
        }
        break;
      case DataMap.Resource_Type.LocalFileSystem.value:
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.tenantName;
        break;
      case DataMap.Resource_Type.LocalLun.value:
        this.removeLinkStatus();
        this.removeType();
        this.formItems[0][0].content = this.source.name;

        break;
      case DataMap.Resource_Type.Dameng.value:
        this.removeType();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.linkStatus === '1' ? 1 : 0;
        break;
      case DataMap.Resource_Type.KubernetesNamespace.value:
      case DataMap.Resource_Type.KubernetesStatefulset.value:
      case DataMap.Resource_Type.kubernetesNamespaceCommon.value:
      case DataMap.Resource_Type.kubernetesDatasetCommon.value: {
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.path;
        this.formItems[0][1].label = this.i18n.get('common_path_label');
        break;
      }
      case DataMap.Resource_Type.HBaseBackupSet.value:
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.environment_name;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        break;
      case DataMap.Resource_Type.ElasticsearchBackupSet.value:
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.environment_name;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        break;
      case DataMap.Resource_Type.HiveBackupSet.value:
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = `${this.source.environment_name}/${this.source.extendInfo?.databaseName}`;
        break;
      case DataMap.Resource_Type.HDFSFileset.value: {
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content =
          this.source.environment?.name || this.source?.environment_name;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        break;
      }
      case DataMap.Resource_Type.OpenGauss_instance.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.owned_instance;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        this.formItems[0][2].content =
          this.source.instanceStatus ===
          DataMap.openGauss_InstanceStatus.normal.value
            ? 1
            : 0;
        break;
      }
      case DataMap.Resource_Type.OpenGauss_database.value: {
        this.removeLinkStatus();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = `${this.source.belong_cluster}/${this.source.owned_instance}`;
        this.formItems[0][1].label = this.i18n.get(
          'commom_owned_instance_label'
        );
        break;
      }
      case DataMap.Resource_Type.Redis.value: {
        this.removeType();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.linkStatus;
        break;
      }
      case DataMap.Resource_Type.ClickHouseCluster.value: {
        this.removeType();
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content =
          this.source.linkStatus === '1' ? '0' : '1';
        break;
      }
      case DataMap.Resource_Type.ClickHouseDatabase.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content =
          this.source.parentName || this.source.parent_name;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        if (!isNil(this.source.environment?.linkStatus)) {
          this.formItems[0][2].content =
            this.source.environment?.linkStatus === '1' ? '0' : '1';
        } else {
          this.formItems[0][2].content =
            this.source.link_status === '1' ? '0' : '1';
        }

        break;
      }
      case DataMap.Resource_Type.ClickHouseTableset.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content =
          this.source.parentName || this.source.parent_name;
        this.formItems[0][1].label = this.i18n.get(
          'insight_report_belong_cluster_label'
        );
        if (!isNil(this.source.environment?.linkStatus)) {
          this.formItems[0][2].content =
            this.source.environment.linkStatus === '1' ? '0' : '1';
        } else {
          this.formItems[0][2].content =
            this.source.link_status === '1' ? '0' : '1';
        }
        break;
      }
      case DataMap.Resource_Type.KingBaseClusterInstance.value:
      case DataMap.Resource_Type.PostgreSQLClusterInstance.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.environment?.name;
        this.formItems[0][1].label = this.i18n.get(
          'protection_host_cluster_name_label'
        );
        this.formItems[0][2].content =
          this.source?.linkStatus === '1' ? '0' : '1';
        break;
      }
      case DataMap.Resource_Type.KingBaseInstance.value:
      case DataMap.Resource_Type.PostgreSQLInstance.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.environment?.name;
        this.formItems[0][1].label = this.i18n.get(
          'protection_host_cluster_name_label'
        );
        this.formItems[0][2].content =
          this.source?.linkStatus === '1' ? '0' : '1';
        break;
      }
      case DataMap.Resource_Type.fusionComputeVirtualMachine.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.environment?.name;
        this.formItems[0][1].label = this.i18n.get('common_location_label');
        this.formItems[0][2].content = this.source?.extendInfo?.status;
        break;
      }
      case DataMap.Resource_Type.CloudHost.value:
      case DataMap.Resource_Type.ApsaraStack.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.environment?.name;
        this.formItems[0][1].label = '';
        this.formItems[0][2].content = this.source?.link_status;
        break;
      }
      case DataMap.Resource_Type.openStackCloudServer.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.path;
        this.formItems[0][1].label = this.i18n.get('common_path_label');
        this.formItems[0][2].content = this.source?.status;
        break;
      }
      case DataMap.Resource_Type.MongoDB.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.path;
        this.formItems[0][1].label = '';
        this.formItems[0][2].content = this.source?.linkStatus;
        break;
      }
      case DataMap.Resource_Type.commonShare.value: {
        this.formItems[0][0].content = this.source.name;
        this.removeType();
        this.removeLinkStatus();
        break;
      }
      case DataMap.Resource_Type.cNwareVm.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.path;
        this.formItems[0][1].label = this.i18n.get('common_location_label');
        this.formItems[0][2].content = this.source?.status;
        break;
      }
      case DataMap.Resource_Type.ExchangeSingle.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.i18n.get(
          DataMap.Resource_Type.ExchangeSingle.label
        );
        this.formItems[0][2].content = this.source?.linkStatus;
        break;
      }
      case DataMap.Resource_Type.ExchangeGroup.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.i18n.get(
          DataMap.Resource_Type.ExchangeGroup.label
        );
        this.formItems[0][2].content = this.source?.linkStatus;
        break;
      }
      case DataMap.Resource_Type.vmGroup.value: {
        this.formItems[0][0].content = this.source.name;
        break;
      }
      case DataMap.Resource_Type.hyperVVm.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.path;
        this.formItems[0][1].label = '';
        this.formItems[0][2].content = this.source?.extendInfo?.State;
        break;
      }
      case DataMap.Resource_Type.saphanaDatabase.value: {
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source.environment?.name;
        this.formItems[0][1].label = '';
        this.formItems[0][2].content = this.source.linkStatus;
        break;
      }
    }
    this.formItems[1][0].content = this.source.protection_status;
    this.formItems[1][1].content = this.source.sla_name;
    this.formItems[1][2].content = this.source.sla_compliance;
  }

  private removeLinkStatus() {
    if (find(this.formItems[0], { key: 'link_status' })) {
      remove(this.formItems[0] as any, { key: 'link_status' });
    }
  }

  private removeType() {
    if (find(this.formItems[0], { key: 'type' })) {
      remove(this.formItems[0] as any, { key: 'type' });
    }
  }
}
