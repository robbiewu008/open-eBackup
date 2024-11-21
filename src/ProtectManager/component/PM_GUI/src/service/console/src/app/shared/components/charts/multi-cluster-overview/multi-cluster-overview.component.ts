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
import { Component, OnInit } from '@angular/core';
import { FormBuilder } from '@angular/forms';
import {
  BaseUtilService,
  ApiMultiClustersService,
  BackupClustersApiService,
  DataMap,
  DataMapService,
  I18NService,
  CAPACITY_UNIT,
  ResourceService,
  StorageUnitService,
  RouterUrl
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { MultiClusterStatus, CookieService } from 'app/shared';
import { Router } from '@angular/router';
import {
  assign,
  each,
  filter,
  find,
  get,
  includes,
  isArray,
  isEmpty,
  reduce,
  round,
  sum
} from 'lodash';
@Component({
  selector: 'aui-multi-cluster-overview',
  templateUrl: './multi-cluster-overview.component.html',
  styleUrls: ['./multi-cluster-overview.component.less']
})
export class MultiClusterOverviewComponent implements OnInit {
  progressBarColor = [[0, '#6C92FA']];
  resourceData = [];
  resourceProtect = null;
  backupStorageUnits = {
    total: 0,
    data: []
  };
  copyList = [];
  archiveList = [];
  abnormalNode = null;
  unitconst = CAPACITY_UNIT;
  isAllCluster = true;
  RouterUrl = RouterUrl;
  round = round;

  constructor(
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public dataMapService: DataMapService,
    private ApiMultiClustersService: ApiMultiClustersService,
    private BackupClustersApiService: BackupClustersApiService,
    private resourceApiService: ResourceService,
    public fb: FormBuilder,
    public router: Router,
    public cookieService: CookieService,
    public appUtilsService: AppUtilsService,
    private storageUnitService: StorageUnitService
  ) {}

  ngOnInit(): void {
    const clusterObj = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isAllCluster =
      (!clusterObj ||
        (clusterObj && clusterObj['icon'] === 'aui-icon-all-cluster')) &&
      !this.cookieService.isCloudBackup;

    const params = { akLoading: false };
    if (this.isAllCluster) {
      assign(params, { isAllCluster: true });
    }
    this.getAbnormalNode();
    this.getResourceInfo();
    this.getBackupInfo(params);
    this.getCopyInfo(params);
    this.getArchiveInfo(params);
    this.resourceData = [
      {
        key: 'Database',
        label: this.i18n.get('common_database_label'),
        protectedCount: 0,
        subType: [...this.appUtilsService.getApplicationConfig().database]
      },
      {
        key: 'BigData',
        label: this.i18n.get('common_bigdata_label'),
        protectedCount: 0,
        subType: [...this.appUtilsService.getApplicationConfig().bigData]
      },
      {
        key: 'Virtualization',
        label: this.i18n.get('common_virtualization_label'),
        protectedCount: 0,
        subType: [...this.appUtilsService.getApplicationConfig().virtualization]
      },
      {
        key: 'Container',
        label: this.i18n.get('common_container_label'),
        protectedCount: 0,
        subType: [...this.appUtilsService.getApplicationConfig().container]
      },
      {
        key: 'Cloud',
        label: this.i18n.get('common_huawei_clouds_label'),
        protectedCount: 0,
        subType: [...this.appUtilsService.getApplicationConfig().cloud]
      },
      {
        key: 'Application',
        label: this.i18n.get('common_application_label'),
        protectedCount: 0,
        subType: [...this.appUtilsService.getApplicationConfig().application]
      },
      {
        key: 'FileSystem',
        label: this.i18n.get('common_file_systems_label'),
        protectedCount: 0,
        subType: [...this.appUtilsService.getApplicationConfig().fileService]
      }
    ];
  }

  //获取多集群资源统计信息
  getResourceInfo() {
    if (this.isAllCluster) {
      this.ApiMultiClustersService.getMultiClusterResources({
        akLoading: false,
        resourceType: 'All'
      }).subscribe(res => {
        this.resourceProtect = res.protectedCount;
        get(res, 'resourceVoList', []);
        each(res?.resourceVoList, item => {
          this.resourceData.map(v => {
            if (v.key === item.resourceType) {
              v.protectedCount = item.protectedCount;
            }
          });
        });
      });
    } else {
      this.resourceApiService
        .summaryProtectionResourceV1ResourceProtectionSummaryGet({
          akLoading: false
        })
        .subscribe(res => {
          let summaryCon = 0;
          get(res, 'summary', []);
          each(this.resourceData, resource => {
            if (resource.subType && !isEmpty(resource.subType)) {
              const resourceProtectedCount = reduce(
                resource.subType,
                (acc, app) => {
                  if (isArray(app.key)) {
                    const filterTmp = filter(res.summary, item =>
                      includes(app.key, item.resource_sub_type)
                    );
                    return (
                      acc +
                      reduce(
                        filterTmp,
                        (count, i) => count + i.protected_count,
                        0
                      )
                    );
                  } else {
                    const findArr = find(
                      res.summary,
                      item => item.resource_sub_type === app.key
                    );
                    return acc + (findArr ? findArr.protected_count : 0);
                  }
                },
                0
              );
              summaryCon += resourceProtectedCount;
              resource.protectedCount = resourceProtectedCount;
            }
          });
          this.resourceProtect = summaryCon;
        });
    }
  }

  //获取备份统计信息
  getBackupInfo(params) {
    this.storageUnitService
      .queryBackUnitGET({
        ...params,
        pageNo: 0,
        pageSize: 3
      })
      .subscribe(res => {
        this.backupStorageUnits.total = res.totalCount;
        this.backupStorageUnits.data = res.records;
      });
  }

  jumpToStorageUnitsPage() {
    this.appUtilsService.isJumpToStorageUnits = true;
    this.router.navigateByUrl(RouterUrl.SystemInfrastructureNasBackupStorage);
  }

  //获取复制统计信息
  getCopyInfo(params) {
    this.ApiMultiClustersService.getMultiClusterReplicationCapacitySummary(
      params
    ).subscribe(res => {
      res.map(item => {
        assign(item, {
          usedPercent: (item.usedCapacity * 100) / item.totalCapacity
        });
        if (item.type === 'replication') {
          let imgSrc = '';
          if (item.totalCapacity) {
            imgSrc = 'assets/img/multi_cluster_backup_normal.png';
          } else {
            imgSrc = 'assets/img/multi_cluster_copy_not_config.png';
          }
          this.copyList.push(
            Object.assign(item, {
              label: this.i18n.get('system_replication_cluster_label'),
              src: imgSrc
            })
          );
        }
      });
    });
  }

  //获取归档统计信息
  getArchiveInfo(params) {
    this.ApiMultiClustersService.getMultiClusterArchiveCapacitySummary(
      params
    ).subscribe(res => {
      res.map(item => {
        assign(item, {
          usedPercent: (item.usedCapacity * 100) / item.totalCapacity
        });
        if (
          item.type === 'tape' &&
          !(
            this.appUtilsService.isDistributed ||
            this.appUtilsService.isDecouple
          )
        ) {
          let imgSrc = '';
          if (item.usedCapacity !== 0) {
            imgSrc = 'assets/img/multi_cluster_tape.png';
          } else {
            imgSrc = 'assets/img/multi_cluster_tape_not_config.png';
          }
          this.archiveList.push(
            Object.assign(item, {
              label: this.i18n.get('system_archive_device_label'),
              src: imgSrc
            })
          );
        }
        if (item.type === 'cloudStorage') {
          let imgSrc = '';
          if (item.totalCapacity) {
            imgSrc = 'assets/img/multi_cluster_cloud.png';
          } else {
            imgSrc = 'assets/img/multi_cluster_cloud_not_config.png';
          }
          this.archiveList.push(
            Object.assign(item, {
              label: this.i18n.get('common_object_storage_label'),
              src: imgSrc
            })
          );
        }
      });
    });
  }

  //获取异常节点数
  getAbnormalNode() {
    this.BackupClustersApiService.getBackupClustersSummary({}).subscribe(
      res => {
        this.abnormalNode = res.wrongClusterNum;
      }
    );
  }

  subTypeClick() {
    MultiClusterStatus.nodeStatus = [
      DataMap.Cluster_Status.offline.value,
      DataMap.Cluster_Status.partOffline.value
    ];
    this.router.navigate(['system/infrastructure/cluster-management']);
  }
}
