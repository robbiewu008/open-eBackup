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
import { Component, OnDestroy, OnInit, Input } from '@angular/core';
import { find } from 'lodash';
import {
  CopiesDetectReportService,
  CommonConsts,
  DataMap,
  CookieService,
  AntiRansomwarePolicyApiService,
  RouterUrl
} from 'app/shared';
import { Subject, Subscription, timer } from 'rxjs';
import { Router } from '@angular/router';
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
    } else if (this.abnormalCount !== 0) {
      return 1;
    } else if (this.detectingCount !== 0) {
      return 4;
    } else {
      return 0;
    }
  }

  RouterUrl = RouterUrl;
  queryTime = Date.now();
  totalCount = 0;
  antiRansomwareType = Object.values(DataMap.antiRansomwareType);
  infectedCounts = [];
  uninfectedCounts = [];
  preparingCounts = [];
  detectingCounts = [];
  uninspectedCounts = [];
  abnormalCounts = [];
  infectedCount = 0;
  uninfectedCount = 0;
  preparingCount = 0;
  detectingCount = 0;
  abnormalCount = 0;
  uninspectedCount = 0;
  completeFlag = false;
  timeSub$: Subscription;
  destroy$ = new Subject();
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  constructor(
    public router: Router,
    private cookieService: CookieService,
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
        resourceSubType: [
          DataMap.Detecting_Resource_Type.virtualMachine.value,
          DataMap.Detecting_Resource_Type.cNwareVm.value,
          DataMap.Detecting_Resource_Type.nasFileSystem.value,
          DataMap.Detecting_Resource_Type.nasShare.value,
          DataMap.Detecting_Resource_Type.fileset.value
        ]
      };
      if (this.timeSub$) {
        this.timeSub$.unsubscribe();
      }
      this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
        .pipe(
          switchMap(index => {
            return this.copiesDetectReportService.ShowDetectionSummary({
              ...params,
              akLoading: false
            });
          }),
          takeUntil(this.destroy$)
        )
        .subscribe(
          res => {
            this.handleDetectionData(res);
            resolve(true);
          },
          () => {
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
    this.abnormalCount = 0; //异常数
    this.uninspectedCount = 0; //未检测数
  }

  handleDetectionData(res) {
    this.resetDetectionData();
    this.queryTime = Date.now();
    this.completeFlag = false;
    this.initCountData(
      find(res, {
        resource_sub_type: DataMap.Detecting_Resource_Type.virtualMachine.value
      })
    );
    this.initCountData(
      find(res, {
        resource_sub_type: DataMap.Detecting_Resource_Type.cNwareVm.value
      })
    );
    this.initCountData(
      find(res, {
        resource_sub_type: DataMap.Detecting_Resource_Type.nasFileSystem.value
      })
    );
    this.initCountData(
      find(res, {
        resource_sub_type: DataMap.Detecting_Resource_Type.nasShare.value
      })
    );
    this.initCountData(
      find(res, {
        resource_sub_type: DataMap.Detecting_Resource_Type.fileset.value
      })
    );
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
    this.abnormalCount += data ? data.abnormal_copy_num : 0;
  }

  getTotalCount() {
    return new Promise(resolve => {
      const params = {
        pageNo: 1,
        pageSize: 1
      };

      this.antiRansomwarePolicyApiService
        .ShowAntiRansomwarePolicies(params)
        .subscribe(res => {
          this.totalCount = res.totalCount;
          resolve(true);
        });
    });
  }

  navigate(params = {}, path = RouterUrl.ExploreAntiRansomware) {
    this.router.navigate([path], {
      queryParams: params
    });
  }
}
