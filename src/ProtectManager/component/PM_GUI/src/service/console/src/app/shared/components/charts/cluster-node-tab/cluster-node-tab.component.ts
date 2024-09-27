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
import {
  ChangeDetectorRef,
  Component,
  EventEmitter,
  OnInit,
  Output
} from '@angular/core';
import {
  CAPACITY_UNIT,
  ClustersApiService,
  BackupClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  GlobalService,
  I18NService
} from 'app/shared';
import {
  assign,
  find,
  first,
  get,
  includes,
  isEmpty,
  isNumber,
  map,
  size
} from 'lodash';
import { AppUtilsService } from '../../../services/app-utils.service';

@Component({
  selector: 'aui-cluster-node-tab',
  templateUrl: './cluster-node-tab.component.html',
  styleUrls: ['./cluster-node-tab.component.less']
})
export class ClusterNodeTabComponent implements OnInit {
  clusterNodes = [];
  showNodeItems = [];
  selectedNode = '';
  start = 0;
  end = 6;
  maxClusterNodeShow = 6;
  dataMap = DataMap;
  selected;
  clusterOps = [];
  isAllCluster = true;
  unitconst = CAPACITY_UNIT;
  isLocalCluster = true;
  curClustersInfo;
  @Output() onChange = new EventEmitter<any>();

  constructor(
    private cdr: ChangeDetectorRef,
    private i18n: I18NService,
    private globalService: GlobalService,
    private clusterApiService: ClustersApiService,
    private BackupClustersApiService: BackupClustersApiService,
    public cookieService: CookieService,
    public appUtilsService?: AppUtilsService
  ) {}

  ngOnInit() {
    this.getCurrentCluster();
    this.getNodes();
  }

  getCurrentCluster() {
    const currentCluster = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isAllCluster =
      isEmpty(currentCluster) || currentCluster?.isAllCluster === true;
    this.isLocalCluster = currentCluster?.clustersType === 1;
    if (this.isAllCluster) {
      this.getClusters();
    }
  }
  getClusters() {
    if (!this.isAllCluster) {
      return;
    }

    this.clusterApiService
      .getClustersInfoUsingGET({
        startPage: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        roleList: [
          DataMap.Target_Cluster_Role.managed.value,
          DataMap.Target_Cluster_Role.management.value
        ],
        statusList: [DataMap.Cluster_Status.online.value]
      })
      .subscribe(res => {
        this.clusterOps = map(res.records, item => {
          return {
            isLeaf: true,
            label: item.clusterName,
            value: item.clusterId,
            ...item
          };
        });
        this.selected = find(res.records, {
          clusterType: DataMap.Cluster_Type.local.value
        })?.clusterId;
        this.curClustersInfo = find(res.records, {
          clusterType: DataMap.Cluster_Type.local.value
        });
        this.cdr.detectChanges();
      });
  }

  getShowClusterItems() {
    this.showNodeItems = this.clusterNodes.slice(this.start, this.end);
  }

  NodeChanged(remoteEsn) {
    const selectedCluster = find(this.clusterOps, {
      clusterId: this.selected
    });
    const info = {
      clusterId: this.selected,
      clusterType: get(selectedCluster, 'clusterType', ''),
      memberEsn: remoteEsn
    };
    this.globalService.emitStore({
      action: 'checkedCluster',
      state: info
    });
  }

  prev() {
    if (!this.start) {
      return;
    }
    this.start--;
    this.end--;
    this.getShowClusterItems();
  }

  next() {
    if (this.end === size(this.clusterNodes)) {
      return;
    }
    this.start++;
    this.end++;
    this.getShowClusterItems();
  }

  changeCluster(item) {
    this.cookieService.setFilterCluster(item);
    this.curClustersInfo = find(this.clusterOps, { clusterId: item });
    if (this.curClustersInfo.clusterType === 1) {
      this.isLocalCluster = true;
    } else {
      this.isLocalCluster = false;
    }
    this.getNodes(this.curClustersInfo.clusterType, item);
  }

  getNodes(type?, id?) {
    this.BackupClustersApiService.queryAllMembers({
      clustersType: type, //本地集群1，其他2
      clustersId: id
    }).subscribe(res => {
      // 排序规则：
      // 第一层:按照节点角色：主节点、备、成员节点
      // 第二层：按照节点状态：在线、设置中、离线、删除中
      const rule = [
        DataMap.Node_Status.online.value,
        DataMap.Node_Status.setting.value,
        DataMap.Node_Status.offline.value,
        DataMap.Node_Status.deleting.value
      ];
      res.sort((a, b) => {
        if (a.role !== b.role) {
          return b.role - a.role;
        } else {
          return rule.indexOf(a.status) - rule.indexOf(b.status);
        }
      });
      this.clusterNodes = res;
      this.getShowClusterItems();
      if (!!size(this.showNodeItems)) {
        this.selectedNode = first(this.showNodeItems).remoteEsn;
        setTimeout(() => {
          this.NodeChanged(this.selectedNode);
        });
      }
    });
  }
}
