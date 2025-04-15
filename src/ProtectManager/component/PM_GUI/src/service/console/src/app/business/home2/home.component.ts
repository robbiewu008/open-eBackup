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
import { DOCUMENT } from '@angular/common';
import { Component, Inject, OnDestroy, OnInit, ViewChild } from '@angular/core';
import {
  BackupClustersApiService,
  ClustersApiService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  RoleType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { includes, reject, set } from 'lodash';
import { forkJoin, of, Subject } from 'rxjs';
import { finalize } from 'rxjs/operators';
import { cardlist } from './cardlist';

const map = new Map();
cardlist.forEach(item => {
  map.set(item.index, item.name);
});
const widthClassMap: any = [
  [736, ['oneColumn', 'wider']],
  [852, ['twoColumns', 'narrower']],
  [1220, ['twoColumns', 'wider']],
  [1289, ['threeColumns', 'narrower']],
  [1643, ['threeColumns', 'wider']],
  [1724, ['fourColumns', 'narrower']],
  [Infinity, ['fourColumns', 'wider']]
];

@Component({
  selector: 'home',
  templateUrl: './home.component.html',
  styleUrls: ['./home.component.less']
})
export class HomeComponent implements OnInit, OnDestroy {
  @ViewChild('missionOverview', { static: false }) missionOverview;
  @ViewChild('performance', { static: false }) performance;
  @ViewChild('capacityDiction', { static: false }) capacityDiction;
  @ViewChild('topFailedTasksSlaProtectionPolicy', { static: false })
  topFailedTasksSlaProtectionPolicy;
  @ViewChild('topFailedTasksResourceObjects', { static: false })
  topFailedTasksResourceObjects;
  @ViewChild('backupSoftwareManagement', { static: false })
  backupSoftwareManagement;
  cardList = cardlist;
  windowResizeObserver = new Subject();
  timer;
  performanceSelectClusterNodes;
  capacityDictionSelectClusterNodes;
  timeOption;
  clusterOption = [];
  clusterNodesOption = [];
  missionOverviewTimeOption;
  performanceTimeOption;
  topTimeOption;
  backupSoftWareTimeOption;
  isNoXSeries = includes(
    [
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );
  isDataBackupOrDecouple = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value,
      DataMap.Deploy_Type.decouple.value
    ],
    this.i18n.get('deploy_type')
  );
  constructor(
    @Inject(DOCUMENT) private document: Document,
    private i18n: I18NService,
    public appUtilsService: AppUtilsService,
    private clusterApiService: ClustersApiService,
    private dataMapService: DataMapService,
    private cookieService: CookieService,
    private BackupClustersApiService: BackupClustersApiService
  ) {}

  ngOnInit(): void {
    if (this.isNoXSeries) {
      return;
    }

    this.initTimeOption();
    this.setResizeListener();
    setTimeout(() => this.setWiderClass());
    this.getClusters();
  }
  initTimeOption() {
    this.timeOption = Object.values(DataMap.homeTimeType).map(item => {
      return { ...item, label: this.i18n.get(item.label) };
    });
    this.missionOverviewTimeOption = this.getTimeOption([0, 3, 4, 5]);
    this.performanceTimeOption = this.getTimeOption([1, 2, 3, 4, 5, 6]);
    this.topTimeOption = this.getTimeOption([4, 5]);
    this.backupSoftWareTimeOption = this.dataMapService.toArray(
      'backupSoftwareTimeType'
    );
  }
  ngOnDestroy() {
    window.removeEventListener('resize', this.resizeHandler);
  }

  setResizeListener() {
    window.addEventListener('resize', this.resizeHandler);
  }
  resizeHandler = () => {
    if (this.timer) {
      clearTimeout(this.timer);
      this.timer = null;
    }
    this.timer = setTimeout(() => {
      this.setWiderClass();
    }, 50);
  };
  setWiderClass() {
    let homeComponent = document.getElementById('homeCpt');
    let width = homeComponent?.getBoundingClientRect().width;
    let classList: Array<string> = widthClassMap.find((item: Array<number>) => {
      return item[0] > width;
    })[1];
    // 清空class重新赋值
    while (homeComponent.classList.length > 0) {
      homeComponent.classList.remove(homeComponent.classList.item(0));
    }
    classList.push('home');
    homeComponent.classList.add(...classList);
  }

  getShowCardList() {
    if (this.appUtilsService.isDistributed) {
      // 分布式不支持数据缩减
      this.cardList = this.cardList.filter(
        (item: any) => ![8, 7, 5].includes(item.index)
      );
    }
    if (this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value) {
      // x3000 不支持防勒索
      this.cardList = this.cardList.filter((item: any) => item.index !== 5);
    }
    // 自定义用户或者数据保护管理员不支持查看DPA纳管
    if (
      ![RoleType.SysAdmin, RoleType.Auditor].includes(this.cookieService.role)
    ) {
      this.cardList = this.cardList.filter((item: any) => item.index !== 13);
    }
    return this.cardList.filter((item: any) => {
      return !item?.show;
    });
  }

  refresh(name) {
    this[name].refreshData();
  }

  getTimeOption(needArr) {
    return this.timeOption.filter(item => {
      return needArr.includes(item.value);
    });
  }

  getClusters() {
    const params = this.isDataBackupOrDecouple
      ? {
          startPage: CommonConsts.PAGE_START,
          pageSize: CommonConsts.PAGE_SIZE,
          roleList: this.appUtilsService.isDecouple
            ? [DataMap.Target_Cluster_Role.backupStorage.value]
            : [
                DataMap.Target_Cluster_Role.management.value,
                DataMap.Target_Cluster_Role.managed.value
              ]
        }
      : {
          startPage: CommonConsts.PAGE_START,
          pageSize: CommonConsts.PAGE_SIZE,
          typeList: [1]
        };
    set(params, 'akLoading', false);
    this.clusterApiService
      .getClustersInfoUsingGET(params)
      .pipe(
        finalize(() => {
          this.clusterOption = [
            {
              isLeaf: true,
              label: this.i18n.get('common_home_all_clusters_label'),
              value: -1
            },
            ...this.clusterOption
          ];
          this.cardList.find(item => {
            return item.name === 'missionOverview';
          }).selectcluster = this.clusterOption[0]?.value;
        })
      )
      .subscribe(res => {
        this.clusterOption = res.records.map(item => {
          return {
            isLeaf: true,
            label: item.clusterName,
            value: item.clusterId,
            ...item
          };
        });
        if (this.appUtilsService.isDecouple) {
          // e1000 性能容量预测不需要展示本地盘
          this.clusterOption = reject(
            this.clusterOption,
            item =>
              item.deviceType === DataMap.poolStorageDeviceType.Server.value
          );
        }
        const nodeRequests = this.clusterOption.map(item => {
          return this.appUtilsService.isDecouple
            ? of(item)
            : this.BackupClustersApiService.queryAllMembers({
                clustersType: item.clusterType,
                clustersId: item.clusterId,
                akLoading: false
              });
        });

        this.requestNodes(nodeRequests);
      });
  }
  requestNodes(nodeRequests) {
    forkJoin(nodeRequests).subscribe((res: any) => {
      this.processClusterOptions(res);
      let performance = this.cardList.find(item => {
        return item.name === 'performance';
      });
      let capacityDiction = this.cardList.find(item => {
        return item.name === 'capacityDiction';
      }) || {
        selectNode: [],
        clusterNodesOptions: [],
        clusterType: undefined
      };
      performance.clusterNodesOptions = this.clusterNodesOption; // 将节点值在card中存一份
      capacityDiction.clusterNodesOptions = this.clusterNodesOption;
      const defaultNode = this.clusterNodesOption[0];
      performance.clusterType = defaultNode?.clusterType;
      capacityDiction.clusterType = defaultNode?.clusterType;
      if (this.appUtilsService.isDecouple) {
        this.performanceSelectClusterNodes = defaultNode?.label;
        this.capacityDictionSelectClusterNodes = defaultNode?.label;
        performance.selectNode = [defaultNode?.value];
        capacityDiction.selectNode = [defaultNode?.value];
      } else {
        this.performanceSelectClusterNodes = defaultNode?.children[0]?.label;
        this.capacityDictionSelectClusterNodes =
          defaultNode?.children[0]?.label;
        performance.selectNode = [
          defaultNode?.value,
          defaultNode?.children[0]?.value
        ];
        capacityDiction.selectNode = [
          defaultNode?.value,
          defaultNode?.children[0]?.value
        ];
      }

      this.refresh('performance');
      if (!this.appUtilsService.isDistributed) {
        this.refresh('capacityDiction');
      }
    });
  }

  private processClusterOptions(res: any) {
    if (this.appUtilsService.isDecouple) {
      this.clusterNodesOption = res.map(item => ({
        ...item,
        children: []
      }));
    } else {
      this.clusterNodesOption = this.clusterOption
        .slice(1)
        .map((item, index) => {
          return {
            ...item,
            isLeaf: false,
            children: this.mapChildren(res, index)
          };
        });
    }
  }

  mapChildren(res, index) {
    return res[index].map(node => {
      return {
        value: node.clusterId,
        label: node.clusterName,
        isLeaf: true,
        ...node
      };
    });
  }

  clusterNodesSelectionChange(values, name, card): void {
    card.clusterType = values[0].clusterType;
    this[`${name}SelectClusterNodes`] = values.pop().label;
    this.refresh(name);
  }
}
