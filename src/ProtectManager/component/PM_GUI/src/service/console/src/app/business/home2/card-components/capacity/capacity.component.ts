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
import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder } from '@angular/forms';
import { Router } from '@angular/router';
import {
  ApiMultiClustersService,
  BackupClustersApiService,
  BaseUtilService,
  CAPACITY_UNIT,
  CapacityApiService,
  CapacityCalculateLabel,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  MultiClusterStatus,
  ResourceService,
  RouterUrl,
  StorageUnitService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  ceil,
  each,
  filter,
  find,
  floor,
  get,
  includes,
  isArray,
  isEmpty,
  isNaN,
  reduce,
  round,
  toNumber
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
    storageNum: 0,
    storageData: [],
    usedCapacity: 0,
    freeCapacity: 0,
    totalCapacity: 0,
    percent: 0
  };
  copyList: { [key: string]: any } = {
    total: 0
  };
  abnormalNode = null;
  unitconst = CAPACITY_UNIT;
  isAllCluster = true;
  RouterUrl = RouterUrl;
  round = round;
  tapeInfo: any = {};
  cloudStorageInfo: any = {};
  showTape = !(
    this.appUtilsService.isDistributed || this.appUtilsService.isDecouple
  );
  isShowOBS = true; // 是否显示对象存储百分比条
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
    private clusterApiService: ClustersApiService,
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
      this.getBackupInfo(params),
      this.getCopyInfo(),
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
          const { records, totalCount } = res;
          const { usedCapacity, totalCapacity } = records.reduce(
            (acc: { usedCapacity; totalCapacity }, item) => {
              if (
                item.deviceType ===
                DataMap.poolStorageDeviceType.OceanProtectX.value
              ) {
                acc.usedCapacity =
                  acc.usedCapacity + Number(item.usedCapacity) / 2; // X系列容量相关要除以2
                acc.totalCapacity =
                  acc.totalCapacity + Number(item.totalCapacity) / 2;
              } else {
                acc.usedCapacity = acc.usedCapacity + Number(item.usedCapacity);
                acc.totalCapacity =
                  acc.totalCapacity + Number(item.totalCapacity);
              }
              return {
                usedCapacity: acc.usedCapacity,
                totalCapacity: acc.totalCapacity
              };
            },
            { usedCapacity: 0, totalCapacity: 0 }
          );
          const freeCapacity = totalCapacity - usedCapacity;
          const unit = CAPACITY_UNIT.KB;
          const percent =
            totalCapacity && ((usedCapacity * 100) / totalCapacity).toFixed(0);
          const { backupStorageUnits } = this;
          assign(backupStorageUnits, {
            storageNum: totalCount,
            totalCapacity: this.transformCapacity(totalCapacity, unit),
            usedCapacity: this.transformCapacity(usedCapacity, unit),
            freeCapacity: this.transformCapacity(freeCapacity, unit),
            percent
          });
          resolve(true);
        });
    });
  }

  //获取复制统计信息
  getCopyInfo() {
    const params = {
      startPage: CommonConsts.PAGE_START,
      roleList: [DataMap.Target_Cluster_Role.replication.value],
      pageSize: CommonConsts.PAGE_SIZE_MAX,
      akLoading: false
    };
    return new Promise(resolve => {
      this.clusterApiService.getClustersInfoUsingGET(params).subscribe(res => {
        resolve(true);
        this.handleCopyInfo(res);
      });
    });
  }

  handleCopyInfo(res) {
    let usedCapacity = 0,
      totalCapacity = 0,
      freeCapacity = 0;
    res.records.forEach(item => {
      if (item.status === DataMap.Cluster_Node_Status.online.value) {
        usedCapacity += toNumber(item.usedCapacity);
        totalCapacity += toNumber(item.capacity);
      }
    });
    freeCapacity = toNumber(totalCapacity - usedCapacity);
    this.copyList = {
      label: this.i18n.get('system_replication_cluster_label'),
      percent: isNaN(usedCapacity / totalCapacity)
        ? 0
        : ceil((usedCapacity * 100) / totalCapacity, 0),
      usedCapacity: this.transformCapacity(usedCapacity),
      freeCapacity: this.transformCapacity(freeCapacity),
      totalCapacity: this.transformCapacity(totalCapacity),
      total: res.totalCount
    };
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
    res.map(item => {
      if (item.type === 'tape' && this.showTape) {
        // 磁带
        this.tapeInfo = Object.assign(item, {
          label: this.i18n.get('system_archive_device_label'),
          usedCapacity: this.transformCapacity(
            item.usedCapacity,
            CAPACITY_UNIT.MB
          )
        });
      }

      if (item.type === 'cloudStorage') {
        // 对象存储
        let percent: number = 100 * (item.usedCapacity / item.totalCapacity);
        assign(item, {
          percent: isNaN(percent) ? 0 : floor(percent, 2)
        });
        this.cloudStorageInfo = Object.assign(item, {
          label: this.i18n.get('common_object_storage_label'),
          usedCapacity: this.transformCapacity(item.usedCapacity),
          freeCapacity: this.transformCapacity(item.freeCapacity),
          totalCapacity: this.transformCapacity(item.totalCapacity)
        });
        if (item.totalCapacity === '-1 KB') {
          // -1情况是az或s3云无法读取容量，且没有其他可读取云
          this.isShowOBS = false;
          this.cloudStorageInfo = Object.assign(item, {
            label: this.i18n.get('common_object_storage_label'),
            usedCapacity: '--',
            freeCapacity: '--',
            totalCapacity: '--'
          });
        }
      }
    });
  }

  //获取异常节点数
  getAbnormalNode() {
    return new Promise(resolve => {
      this.BackupClustersApiService.getBackupClustersSummary({
        akLoading: false
      }).subscribe(res => {
        this.abnormalNode = res.wrongClusterNum;
        resolve(true);
      });
    });
  }

  subTypeClick() {
    MultiClusterStatus.nodeStatus = [
      DataMap.Cluster_Status.offline.value,
      DataMap.Cluster_Status.partOffline.value
    ];
    this.router.navigate([RouterUrl.SystemInfrastructureClusterManagement]);
  }

  private transformCapacity(capacity, unit = CAPACITY_UNIT.KB) {
    return this.capacityCalculateLabel.transform(
      capacity,
      '1.0-2',
      unit,
      false
    );
  }

  navigate(url, params?) {
    this.router.navigate(url, params);
  }
}
