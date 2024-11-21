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
import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import {
  AntiRansomwarePolicyApiService,
  CommonConsts,
  CookieService,
  CopiesDetectReportService,
  DataMap,
  DataMapService,
  RouterUrl
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { each } from 'lodash';
import { forkJoin, Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';

interface Resources {
  key: string;
  name: string;
  type: string;
  abnormal: number;
  infected: number;
}

@Component({
  selector: 'anti-ransomware',
  templateUrl: './anti-ransomware.component.html',
  styleUrls: ['./anti-ransomware.component.less']
})
export class AntiRansomwareComponent implements OnInit, OnDestroy {
  @Input() cardInfo: any = {};
  data = [];

  get type() {
    if (!this.completeFlag) {
      return 3;
    } else if (this.infectedCount !== 0) {
      return 2;
    } else if (this.detectingCount !== 0) {
      return 4;
    } else {
      return 0;
    }
  }
  DataMap = DataMap;
  RouterUrl = RouterUrl;
  queryTime = Date.now();
  totalCount = 0;
  resourceType = this.dataMapService
    .toArray('Detecting_Resource_Type')
    .map(item => item.value);
  antiRansomwareType = Object.values(DataMap.antiRansomwareType);
  infectedCounts = [];
  uninfectedCounts = [];
  preparingCounts = [];
  detectingCounts = [];
  uninspectedCounts = [];
  abnormalCounts = [];
  detectResourceCount = 0;
  infectedCount = 0;
  uninfectedCount = 0;
  preparingCount = 0;
  detectingCount = 0;
  uninspectedCount = 0;
  completeFlag = false;
  timeSub$: Subscription;
  destroy$ = new Subject();
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  constructor(
    public router: Router,
    private appUtilsService: AppUtilsService,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private copiesDetectReportService: CopiesDetectReportService,
    private antiRansomwarePolicyApiService: AntiRansomwarePolicyApiService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.cardInfo.loading = true;
    Promise.all([this.getDetectionData(), this.getTotalCount()]).then(() => {
      this.cardInfo.loading = false;
    });
  }

  onChange() {
    this.ngOnInit();
  }

  getDetectionData() {
    return new Promise(resolve => {
      const params = {
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        akLoading: false,
        resourceSubType: this.resourceType
      };
      if (this.timeSub$) {
        this.timeSub$.unsubscribe();
      }
      this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL_RESOURCE)
        .pipe(
          switchMap(index => {
            const statsObservable = this.copiesDetectReportService.ShowDetectionStatistics(
              { ...params, resourceSubType: this.resourceType.join(',') }
            );
            const summaryObservable = this.copiesDetectReportService.ShowDetectionSummary(
              { ...params }
            );
            return forkJoin({
              resourceSummary: statsObservable,
              detectionSummary: summaryObservable
            });
          }),
          takeUntil(this.destroy$)
        )
        .subscribe(
          res => {
            this.detectResourceCount = res.resourceSummary.total;
            this.handleDetectionData(res.detectionSummary);
            resolve(true);
          },
          () => {
            this.detectResourceCount = 0;
            this.resetDetectionData();
            this.completeFlag = true;
            resolve(true);
          }
        );
    });
  }

  resetDetectionData() {
    this.infectedCounts = [];
    this.uninfectedCounts = [];
    this.preparingCounts = [];
    this.detectingCounts = [];
    this.uninspectedCounts = [];
    this.abnormalCounts = [];
    this.infectedCount = 0; //感染数
    this.uninfectedCount = 0; //未感染数
    this.preparingCount = 0; //准备中数
    this.detectingCount = 0; //检测中数量
    this.uninspectedCount = 0; //未检测数
  }

  handleDetectionData(res) {
    this.resetDetectionData();
    this.queryTime = Date.now();
    this.completeFlag = false;
    each(res, item => this.initCountData(item));
    this.completeFlag = true;
  }

  initCountData(data) {
    this.infectedCounts = this.infectedCounts.concat([
      data ? data.infected_copy_num : 0
    ]);
    this.uninfectedCounts = this.uninfectedCounts.concat([
      data ? data.uninfected_copy_num : 0
    ]);
    this.preparingCounts = this.preparingCounts.concat([
      data ? data.prepare_copy_num : 0
    ]);
    this.detectingCounts = this.detectingCounts.concat([
      data ? data.detecting_copy_num : 0
    ]);
    this.uninspectedCounts = this.uninspectedCounts.concat([
      data ? data.uninspected_copy_num : 0
    ]);
    this.abnormalCounts = this.abnormalCounts.concat([
      data ? data.abnormal_copy_num : 0
    ]);
    this.infectedCount += data ? data.infected_copy_num : 0;
    this.uninfectedCount += data ? data.uninfected_copy_num : 0;
    this.preparingCount += data ? data.prepare_copy_num : 0;
    this.detectingCount += data ? data.detecting_copy_num : 0;
    this.uninspectedCount += data ? data.uninspected_copy_num : 0;
  }

  getTotalCount() {
    return new Promise(resolve => {
      const params = {
        pageNo: 1,
        pageSize: 1,
        akLoading: false
      };

      this.antiRansomwarePolicyApiService
        .ShowAntiRansomwarePolicies(params)
        .subscribe(res => {
          this.totalCount = res.totalCount;
          resolve(true);
        });
    });
  }

  navigate(params = null, path = RouterUrl.ExploreAntiRansomware) {
    if (params) {
      this.appUtilsService.setCacheValue('jump-to-anti-ransomware', params);
    }
    this.router.navigate([path]);
  }
}
