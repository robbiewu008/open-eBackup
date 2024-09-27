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
  Component,
  OnDestroy,
  OnInit,
  Output,
  EventEmitter,
  ChangeDetectionStrategy,
  ChangeDetectorRef
} from '@angular/core';
import { ClustersApiService, CookieService } from 'app/shared';
import { CommonConsts, DataMap, RoleType } from 'app/shared/consts';
import { assign, map, isEmpty, find } from 'lodash';

@Component({
  selector: 'aui-multi-cluster-switch',
  templateUrl: './multi-cluster-switch.component.html',
  styleUrls: ['./multi-cluster-switch.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class MultiClusterSwitchComponent implements OnInit, OnDestroy {
  cluster;
  options = [];
  roleType = RoleType;
  isAllCluster = true;

  @Output() onChange = new EventEmitter<any>();

  constructor(
    private cdr: ChangeDetectorRef,
    public cookieService: CookieService,
    private clusterApiService: ClustersApiService
  ) {}

  ngOnInit() {
    this.getCurrentCluster();
    this.getClusters();
  }

  getCurrentCluster() {
    const currentCluster = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isAllCluster =
      isEmpty(currentCluster) || currentCluster?.isAllCluster === true;
    if (this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE) {
      this.isAllCluster = false;
    }
    // 多集群场景不再支持子页面内切换集群
    this.isAllCluster = false;
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
        this.options = map(res.records, item => {
          return assign(item, {
            key: item.clusterId,
            value: item.clusterId,
            label: item.clusterName,
            isLeaf: true
          });
        });
        this.cluster = find(res.records, {
          clusterType: DataMap.Cluster_Type.local.value
        });
        this.cdr.detectChanges();
      });
  }

  change(item) {
    this.cookieService.setFilterCluster(item);
    this.onChange.emit();
  }

  ngOnDestroy() {
    this.cookieService.setFilterCluster(null);
  }
}
