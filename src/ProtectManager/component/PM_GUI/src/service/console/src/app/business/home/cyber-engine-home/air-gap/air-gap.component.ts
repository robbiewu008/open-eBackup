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
import { each, set } from 'lodash';
import {
  AntiRansomwareAirgapApiService,
  CommonConsts,
  DataMapService,
  I18NService,
  RouterUrl
} from 'app/shared';
import { AirGapDeviceInfo } from 'app/shared/api/models/air-gap-device-info';
import { Router } from '@angular/router';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'app-air-gap',
  templateUrl: './air-gap.component.html',
  styleUrls: ['./air-gap.component.less']
})
export class AirGapComponent implements OnInit {
  total: number = 0;
  dataList: AirGapDeviceInfo[] = [];
  statusConfig = this.dataMapService.getConfig('airgapLinkStatus', true);
  constructor(
    private antiRansomwareAirgapApiService: AntiRansomwareAirgapApiService,
    private dataMapService: DataMapService,
    public i18n: I18NService,
    private router: Router,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.loadData();
    each(this.statusConfig, item => {
      set(item, 'label', this.i18n.get(item.label));
    });
  }

  loadData(loading: boolean = true) {
    this.antiRansomwareAirgapApiService
      .ShowPageDevices({
        pageSize: CommonConsts.PAGE_SIZE,
        pageNo: CommonConsts.PAGE_START,
        akLoading: loading
      })
      .subscribe(res => {
        this.total = res.totalCount ?? 0;
        this.dataList = res.records ?? [];
      });
  }

  gotoAirGap($event, item?) {
    if ($event?.stopPropagation) {
      $event?.stopPropagation();
    }
    if (item) {
      this.appUtilsService.setCacheValue('airgapFilter', item?.name);
    }
    this.router.navigateByUrl(RouterUrl.ExplorePolicyAirgap);
  }

  gotoStorageDevice($event) {
    if ($event?.stopPropagation) {
      $event?.stopPropagation();
    }
    this.router.navigateByUrl(RouterUrl.ExploreStorageDevice);
  }
}
