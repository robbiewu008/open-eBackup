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
import { Component, OnInit, Input } from '@angular/core';
import { FormBuilder } from '@angular/forms';
import {
  BaseUtilService,
  CapacityApiService,
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

import { CapacityCalculateLabel } from 'app/shared';
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
  round
} from 'lodash';

@Component({
  selector: 'capacity',
  templateUrl: './capacity.component.html',
  styleUrls: ['./capacity.component.less'],
  providers: [CapacityCalculateLabel]
})
export class CapacityComponent implements OnInit {
  @Input() cardInfo: any = {};
  progressBarColor = [[0, '#6C92FA']];
  resourceData = [];
  resourceProtect = null;
  backupStorageUnits = {
    total: 0,
    data: []
  };
  copyList = [];
  abnormalNode = null;
  unitconst = CAPACITY_UNIT;
  isAllCluster = true;
  RouterUrl = RouterUrl;
  round = round;
  backupCapacityInfo: any = {
    percent: 0
  };
  tapeInfo: any = {};
  cloudStorageInfo: any = {};
  hasArchiving = false;

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
    private storageUnitService: StorageUnitService,
    public capacityApiService: CapacityApiService,
    public capacityCalculateLabel: CapacityCalculateLabel
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
    this.initResourceData();
    this.cardInfo.loading = true;
    Promise.all([
      this.getAbnormalNode(),
      this.getResourceInfo(),
      this.getBackupCapcacity(),
      this.getBackupInfo(params),
      this.getCopyInfo(params),
      this.getArchiveInfo(params)
    ]).then(() => {
      this.cardInfo.loading = false;
    });
  }

  initResourceData() {
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
        key: 'FileService',
        label: this.i18n.get('common_file_service_label'),
        protectedCount: 0,
        subType: [...this.appUtilsService.getApplicationConfig().fileService]
      },
      {
        key: 'BareMetal',
        label: this.i18n.get('common_bare_metal_label'),
        protectedCount: 0,
        subType: [...this.appUtilsService.getApplicationConfig().bareMetal]
      }
    ];
  }

  //获取多集群资源统计信息
  getResourceInfo() {
    return new Promise(resolve => {
      if (this.isAllCluster) {
        this.ApiMultiClustersService.getMultiClusterResources({
          akLoading: false,
          resourceType: 'All'
        }).subscribe(res => {
          this.handleAllClusterResourceInfo(res);
          resolve(true);
        });
      } else {
        this.resourceApiService
          .summaryProtectionResourceV1ResourceProtectionSummaryGet({
            akLoading: false
          })
          .subscribe(res => {
            this.handleResourceInfo(res);
            resolve(true);
          });
      }
    });
  }
  handleAllClusterResourceInfo(res) {
    this.resourceProtect = res.protectedCount;
    get(res, 'resourceVoList', []);
    each(res?.resourceVoList, item => {
      this.resourceData.map(v => {
        if (v.key === item.resourceType) {
          v.protectedCount = item.protectedCount;
        }
      });
    });
  }

  handleResourceInfo(res) {
    let summaryCon = 0;
    get(res, 'summary', []);
    each(this.resourceData, resource => {
      if (resource.subType && !isEmpty(resource.subType)) {
        const resourceProtectedCount = this.getResourceProtectedCount(
          resource,
          res
        );
        summaryCon += resourceProtectedCount;
        resource.protectedCount = resourceProtectedCount;
      }
    });
    this.resourceProtect = summaryCon;
  }

  getResourceProtectedCount(resource, res) {
    return reduce(
      resource.subType,
      (acc, app) => {
        if (isArray(app.key)) {
          const filterTmp = filter(res.summary, item =>
            includes(app.key, item.resource_sub_type)
          );
          return (
            acc + reduce(filterTmp, (count, i) => count + i.protected_count, 0)
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
  }

  //获取备份统计信息
  getBackupInfo(params) {
    return new Promise(resolve => {
      this.storageUnitService
        .queryBackUnitGET({
          ...params,
          pageNo: 0
        })
        .subscribe(res => {
          this.backupStorageUnits.total = res.totalCount;
          this.backupStorageUnits.data = res.records;
          resolve(true);
        });
    });
  }

  //获取复制统计信息
  getCopyInfo(params) {
    return new Promise(resolve => {
      this.ApiMultiClustersService.getMultiClusterReplicationCapacitySummary(
        params
      ).subscribe(res => {
        this.handleCopyInfo(res);
        resolve(true);
      });
    });
  }

  handleCopyInfo(res) {
    res.map(item => {
      assign(item, {
        percent:
          ((item.usedCapacity * 100) / item.totalCapacity).toFixed(0) || 0
      });
      if (item.type === 'replication' && item.totalCapacity !== 0) {
        //为0视为未配置
        this.copyList.push(
          Object.assign(item, {
            label: this.i18n.get('system_replication_cluster_label'),
            usedCapacity: this.transformKBUnit(item.usedCapacity),
            freeCapacity: this.transformKBUnit(item.freeCapacity),
            totalCapacity: this.transformKBUnit(item.totalCapacity)
          })
        );
      }
    });
  }

  //获取归档统计信息
  getArchiveInfo(params) {
    return new Promise(resolve => {
      this.ApiMultiClustersService.getMultiClusterArchiveCapacitySummary(
        params
      ).subscribe(res => {
        this.handleArchiveInfo(res);
        resolve(true);
      });
    });
  }

  handleArchiveInfo(res) {
    this.hasArchiving = false;
    res.map(item => {
      assign(item, {
        percent:
          (100 * (item.usedCapacity / item.totalCapacity)).toFixed(0) || 0
      });
      if (
        item.type === 'tape' &&
        !(
          this.appUtilsService.isDistributed || this.appUtilsService.isDecouple
        ) &&
        item.usedCapacity !== 0
      ) {
        // 磁带
        this.hasArchiving = true;
      }
      if (
        item.type === 'tape' &&
        !(this.appUtilsService.isDistributed || this.appUtilsService.isDecouple)
      ) {
        // 磁带
        this.tapeInfo = Object.assign(item, {
          label: this.i18n.get('system_archive_device_label'),
          usedCapacity: this.transformKBUnit(item.usedCapacity)
        });
      }

      if (item.type === 'cloudStorage' && item.totalCapacity !== 0) {
        // 对象存储
        this.hasArchiving = true;
      }
      if (item.type === 'cloudStorage') {
        // 对象存储
        this.cloudStorageInfo = Object.assign(item, {
          label: this.i18n.get('common_object_storage_label'),
          usedCapacity: this.transformKBUnit(item.usedCapacity),
          freeCapacity: this.transformKBUnit(item.freeCapacity),
          totalCapacity: this.transformKBUnit(item.totalCapacity)
        });
      }
    });
  }

  //获取异常节点数
  getAbnormalNode() {
    return new Promise(resolve => {
      this.BackupClustersApiService.getBackupClustersSummary({}).subscribe(
        res => {
          this.abnormalNode = res.wrongClusterNum;
          resolve(true);
        }
      );
    });
  }

  subTypeClick() {
    MultiClusterStatus.nodeStatus = [
      DataMap.Cluster_Status.offline.value,
      DataMap.Cluster_Status.partOffline.value
    ];
    this.router.navigate([RouterUrl.SystemInfrastructureClusterManagement]);
  }

  getBackupCapcacity() {
    if (this.cookieService.isCloudBackup) {
      return;
    }
    return new Promise(resolve => {
      if (this.isAllCluster) {
        this.ApiMultiClustersService.getMultiClusterCapacity({
          akLoading: false
        }).subscribe(res => {
          this.backupCapacityInfo = {
            usedCapacity: this.transformKBUnit(res.usedCapacity),
            freeCapacity: this.transformKBUnit(res.freeCapacity),
            totalCapacity: this.transformKBUnit(res.totalCapacity),
            percent:
              (100 * (res.usedCapacity / res.totalCapacity)).toFixed(0) || 0
          };
          resolve(true);
        });
      } else {
        this.capacityApiService
          .queryClusterStorageUsingGET({ akLoading: false })
          .subscribe(res => {
            this.backupCapacityInfo = {
              usedCapacity: this.transformKBUnit(res.usedCapacity),
              freeCapacity: this.transformKBUnit(res.freeCapacity),
              totalCapacity: this.transformKBUnit(res.totalCapacity),
              percent:
                (100 * (res.usedCapacity / res.totalCapacity)).toFixed(0) || 0
            };
            resolve(true);
          });
      }
    });
  }

  private transformKBUnit(capacity) {
    return this.capacityCalculateLabel.transform(
      capacity,
      '1.0-2',
      CAPACITY_UNIT.KB,
      false
    );
  }

  navigate(url, params?) {
    this.router.navigate(url, params);
  }
}
