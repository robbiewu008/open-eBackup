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
  DataMap,
  I18NService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { includes } from 'lodash';
import { forkJoin, Subject } from 'rxjs';
import { cardlist } from './cardlist';

const map = new Map();
cardlist.forEach(item => {
  map.set(item.index, item.name);
});
const widthClassMap: any = [
  [736, ['oneColumn', 'wider']],
  [852, ['twoColumns', 'narrower']],
  [1112, ['twoColumns', 'wider']],
  [1289, ['threeColumns', 'narrower']],
  [1488, ['threeColumns', 'wider']],
  [1724, ['fourColumns', 'narrower']],
  [Infinity, ['fourColumns', 'wider']]
];

@Component({
  selector: 'home',
  templateUrl: './home.component.html',
  styleUrls: ['./home.component.less']
})
export class HomeComponent implements OnInit, OnDestroy {
  private linkElement: HTMLLinkElement;
  @ViewChild('missionOverview', { static: false }) missionOverview;
  @ViewChild('performance', { static: false }) performance;
  @ViewChild('capacityDiction', { static: false }) capacityDiction;
  @ViewChild('topFailedTasksSlaProtectionPolicy', { static: false })
  topFailedTasksSlaProtectionPolicy;
  @ViewChild('topFailedTasksResourceObjects', { static: false })
  topFailedTasksResourceObjects;
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
  isNoXSeries = includes(
    [
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );

  constructor(
    @Inject(DOCUMENT) private document: Document,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private clusterApiService: ClustersApiService,
    private BackupClustersApiService: BackupClustersApiService
  ) {}

  ngOnInit(): void {
    if (this.isNoXSeries) {
      return;
    }
    this.linkElement = this.document.createElement('link');
    this.linkElement.rel = 'stylesheet';
    this.linkElement.href = 'assets/style/aui-dark.min.css';
    this.document.head.appendChild(this.linkElement);

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
  }
  ngOnDestroy() {
    if (this.linkElement && this.linkElement.parentNode) {
      this.linkElement.parentNode.removeChild(this.linkElement);
    }
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
        (item: any) => ![8, 5].includes(item.index)
      );
    }
    if (this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value) {
      // x3000 不支持防勒索
      this.cardList = this.cardList.filter((item: any) => item.index !== 5);
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
        this.clusterOption = res.records.map(item => {
          return {
            isLeaf: true,
            label: item.clusterName,
            value: item.clusterId,
            ...item
          };
        });
        this.cardList.find(item => {
          return (item.name = 'missionOverview');
        }).selectcluster = this.clusterOption[0]?.value;

        const nodeRequests = this.clusterOption.map(item => {
          return this.BackupClustersApiService.queryAllMembers({
            clustersType: item.clustersType,
            clustersId: item.clustersId
          });
        });

        this.requestNodes(nodeRequests);
      });
  }
  requestNodes(nodeRequests) {
    forkJoin(nodeRequests).subscribe((res: any) => {
      this.clusterNodesOption = this.clusterOption.map((item, index) => {
        return {
          ...item,
          isLeaf: false,
          children: this.mapChildren(res, index)
        };
      });
      this.performanceSelectClusterNodes = this.clusterNodesOption[0]?.children[0]?.label;
      this.capacityDictionSelectClusterNodes = this.clusterNodesOption[0]?.children[0]?.label;
      let performance = this.cardList.find(item => {
        return item.name === 'performance';
      });
      let capacityDiction = this.cardList.find(item => {
        return item.name === 'capacityDiction';
      });
      performance.selectNode = [
        this.clusterNodesOption[0]?.value,
        this.clusterNodesOption[0]?.children[0].value
      ];
      capacityDiction.selectNode = [
        this.clusterNodesOption[0]?.value,
        this.clusterNodesOption[0]?.children[0].value
      ];
      performance.clusterType = this.clusterNodesOption[0]?.clusterType;
      capacityDiction.clusterType = this.clusterNodesOption[0]?.clusterType;

      this.refresh('performance');
      this.refresh('capacityDiction');
      this.clusterOption = [
        {
          isLeaf: true,
          label: '全部集群',
          value: -1
        },
        ...this.clusterOption
      ];
    });
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

  clusterNodesSelectionChange(values, name): void {
    this.refresh(name);
    this[`${name}SelectClusterNodes`] = values.pop().label;
  }
}
