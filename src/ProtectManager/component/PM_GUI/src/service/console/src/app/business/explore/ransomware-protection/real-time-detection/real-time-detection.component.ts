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
import { IODETECTFILESYSTEMService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-real-time-detection',
  templateUrl: './real-time-detection.component.html',
  styleUrls: ['./real-time-detection.component.less']
})
export class RealTimeDetectionComponent implements OnInit {
  activeIndex;
  totalFileSystem = 0;
  protectedFileSystem = 0;
  totalPolicy = 0;
  totalWhitelist = 0;

  constructor(
    private appUtilsService: AppUtilsService,
    private ioDetectFilesystemService: IODETECTFILESYSTEMService
  ) {}

  ngOnInit(): void {
    if (this.appUtilsService.getCacheValue('ioDetectionTab')) {
      this.activeIndex = 'policy';
    }
    this.getSummary();
  }

  getSummary(loading = true) {
    this.ioDetectFilesystemService
      .getIoDetectConfigSummary({ akLoading: loading })
      .subscribe(res => {
        this.totalFileSystem = res.fsNum;
        this.protectedFileSystem = res.ioDetectEnabledNum;
        this.totalPolicy = res.policyNum;
        this.totalWhitelist = res.whitelistNum;
      });
  }

  refreshSummary() {
    this.getSummary(false);
  }
}
