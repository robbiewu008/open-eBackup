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
import { Router } from '@angular/router';
import {
  ConfigManagementService,
  I18NService,
  SupportLicense
} from 'app/shared';

@Component({
  selector: 'aui-detection-statistics',
  templateUrl: './detection-statistics.component.html',
  styleUrls: ['./detection-statistics.component.less']
})
export class DetectionStatisticsComponent implements OnInit {
  enableBlockCount = 0;
  disableBlockCount = 0;
  enableDetectionCount = 0;
  disableDetectionCount = 0;
  enableSnapshotCount = 0;
  disableSnapshotCount = 0;
  dectionTotalCount = 0;
  snapshotTotalCount = 0;
  sanIoEnableCount = 0;
  sanCopyEnableCount = 0;
  sanIoTotalCount = 0;
  sanCopyTotalCount = 0;
  fileTotalCount = 0;
  blockStatusLabel = this.i18n.get('common_disabled_label');
  supportLicense = SupportLicense;

  constructor(
    public router: Router,
    private i18n: I18NService,
    private configManagementService: ConfigManagementService
  ) {}

  ngOnInit() {
    this.updateData();
  }

  getDetail() {
    this.router.navigate(['explore/anti-ransomware/detection-setting']);
  }

  updateData() {
    this.configManagementService
      .getVstoreDetectConfigsSummary({})
      .subscribe(res => {
        this.enableBlockCount =
          res?.fileExtensionFilterConfigSummary?.enabledCount;
        this.disableBlockCount =
          res?.fileExtensionFilterConfigSummary?.disabledCount;
        this.fileTotalCount = this.enableBlockCount + this.disableBlockCount;

        this.enableDetectionCount = res?.ioDetectConfigSummary?.enabledCount;
        this.disableDetectionCount = res?.ioDetectConfigSummary?.disabledCount;
        this.dectionTotalCount =
          this.enableDetectionCount + this.disableDetectionCount;
        this.enableSnapshotCount = res?.copyDetectConfigSummary?.enabledCount;
        this.disableSnapshotCount = res?.copyDetectConfigSummary?.disabledCount;
        this.snapshotTotalCount =
          this.enableSnapshotCount + this.disableSnapshotCount;

        this.sanIoEnableCount =
          res?.sanIoDetectConfigSummary?.enabledCount || 0;
        this.sanCopyEnableCount = res?.sanCopyDetectConfigSummary?.enabledCount;
        this.sanIoTotalCount =
          res?.sanIoDetectConfigSummary?.enabledCount +
          res?.sanIoDetectConfigSummary?.disabledCount;
        this.sanCopyTotalCount =
          res?.sanCopyDetectConfigSummary?.enabledCount +
          res?.sanCopyDetectConfigSummary?.disabledCount;
      });
  }
}
