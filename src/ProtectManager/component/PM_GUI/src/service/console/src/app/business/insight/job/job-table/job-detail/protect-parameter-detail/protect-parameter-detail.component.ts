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
import { ApplicationType, DataMap } from 'app/shared';
import { ProtectedResourceApiService } from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, findKey, get, isArray, isNumber } from 'lodash';

@Component({
  selector: 'aui-protect-parameter-detail',
  templateUrl: './protect-parameter-detail.component.html',
  styleUrls: ['./protect-parameter-detail.component.less']
})
export class ProtectParameterDetailComponent implements OnInit {
  @Input() job;
  data: any = {};
  application;
  applicationType = ApplicationType;
  dataMap = DataMap;
  String = String;
  isProxy = false; // 是否有代理主机
  isSmallFile = false; // 是否有小文件聚合
  isChannel = false; // 是否有通道数
  isDeleteLog = false; // 是否备份完成后删除归档日志
  agentName;
  osType; // 用于文件级操作系统判断
  isObjectShowBucket = false; // 用于对象存储桶日志是否展示
  isConcurrent = false; // 用于并发数展示
  extendInfo;
  resourceSubTypeAppMap = this.appUtilsService.findResourceTypeByKey('slaId');

  constructor(
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.extendInfo = JSON.parse(this.job?.extendStr || '{}');
    this.data = this.extendInfo?.triggerPolicy?.protected_obj_ext_param;
    this.application = this.getTypeKey(
      this.job.sourceSubType,
      this.resourceSubTypeAppMap
    );
    this.isSmallFile = [ApplicationType.Fileset].includes(this.application);
    this.isProxy = [
      ApplicationType.Vmware,
      ApplicationType.CNware,
      ApplicationType.FusionCompute,
      ApplicationType.KubernetesStatefulSet,
      ApplicationType.KubernetesDatasetCommon,
      ApplicationType.HCSCloudHost,
      ApplicationType.OpenStack,
      ApplicationType.ApsaraStack,
      ApplicationType.NASFileSystem
    ].includes(this.application);
    if (
      (this.job.sourceSubType === ApplicationType.Oracle &&
        !!this.data?.storage_snapshot_flag) ||
      this.job.sourceSubType === DataMap.Resource_Type.oracleCluster.value
    ) {
      // oracle单机数据库的主机需要先开存储层快照备份
      this.isProxy = true;
    }
    if (
      this.application === ApplicationType.Oracle &&
      !!this.data?.storage_snapshot_flag
    ) {
      this.isConcurrent = true;
    }
    this.isChannel = [
      ApplicationType.Fileset,
      ApplicationType.NASShare
    ].includes(this.application);
    this.isDeleteLog = [ApplicationType.Oracle, ApplicationType.TDSQL].includes(
      this.application
    );

    if (this.isProxy && !!this.data?.agents) {
      this.getAgent();
    }

    if (this.application === ApplicationType.Vmware && !!this.data?.host_list) {
      // vmware的字段不一样
      this.agentName = this.data.host_list
        .map(item => `${item.name}(${item.host})`)
        .join(', ');
    }

    if (
      this.job.sourceSubType === ApplicationType.Oracle &&
      !!this.data?.snapshot_agents
    ) {
      // oracle单机代理主机
      assign(this.data, {
        agents: this.data.snapshot_agents
      });
      this.getAgent();
    }

    if (this.application === ApplicationType.Fileset) {
      this.osType = this.extendInfo?.jobConfig?.osType;
    }

    if (this.application === ApplicationType.ObjectStorage) {
      // 对象存储只有HCS能展示桶日志增量
      this.isObjectShowBucket =
        Number(this.extendInfo?.storageType) ===
        DataMap.objectStorageType.hcs.value;
    }
    this.formatCustomData();
  }

  getAgent() {
    this.protectedResourceApiService
      .ListResources({
        pageNo: 0,
        pageSize: this.data?.agents.split(';')?.length || 200,
        conditions: JSON.stringify({
          uuid: this.data.agents.split(';')
        }),
        akDoException: false
      })
      .subscribe((res: any) => {
        if (!res?.records?.length) {
          return;
        }
        this.agentName = res.records
          .map(item => `${item.name}(${item.endpoint})`)
          .join(', ');
      });
  }

  protected readonly Boolean = Boolean;

  formatCustomData() {
    // 自定义数据展示
    if (this.application === ApplicationType.FusionCompute) {
      const delete_speed = get(this.data, 'snap_delete_speed', 0);
      // 手动填写范围是10-500 为0则说明没填写
      if (isNumber(delete_speed) && !delete_speed) {
        // 如果保护时高级参数不指定快照删除速率则默认展示30MB/s
        this.data.snap_delete_speed = 30;
      }
    }
  }

  /**
   * 根据subType取出对应的AppType
   * @param type
   * @param typeMap
   */
  getTypeKey(type: string, typeMap) {
    const key = findKey(typeMap, item => {
      if (isArray(item)) {
        return item.includes(type);
      } else {
        return item === type;
      }
    });
    return key || type;
  }
}
