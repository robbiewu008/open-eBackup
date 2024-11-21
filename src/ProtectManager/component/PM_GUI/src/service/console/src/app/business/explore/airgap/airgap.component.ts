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
import {
  AntiRansomwareAirgapApiService,
  CommonConsts,
  DataMap,
  I18NService
} from 'app/shared';

@Component({
  selector: 'aui-airgap',
  templateUrl: './airgap.component.html',
  styleUrls: ['./airgap.component.less']
})
export class AirgapComponent implements OnInit {
  activeIndex = 'storage';
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  totalDevice = 0;
  totalPolicy = 0;

  constructor(
    private i18n: I18NService,
    private antiRansomwareAirgapApiService: AntiRansomwareAirgapApiService
  ) {}

  ngOnInit(): void {
    this.getDevice();
    this.getPolicy();
  }

  getDevice(mask = true) {
    if (!this.isCyberEngine) {
      return;
    }
    const params = {
      pageNum: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      akLoading: mask
    };
    this.antiRansomwareAirgapApiService
      .ShowPageDevices(params)
      .subscribe(res => {
        this.totalDevice = res.totalCount;
      });
  }

  getPolicy(mask = true) {
    if (!this.isCyberEngine) {
      return;
    }
    const params = {
      pageNum: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      akLoading: mask
    };
    this.antiRansomwareAirgapApiService
      .ShowPagePolicies(params)
      .subscribe(res => {
        this.totalPolicy = res.totalCount;
      });
  }

  refreshDevice() {
    this.getDevice(false);
  }

  refreshPolicy() {
    this.getPolicy(false);
  }
}
