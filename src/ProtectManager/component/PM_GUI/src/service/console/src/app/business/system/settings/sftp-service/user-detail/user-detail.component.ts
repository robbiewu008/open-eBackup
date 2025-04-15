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
  CAPACITY_UNIT,
  SftpManagerApiService,
  CapacityCalculateLabel
} from 'app/shared';

@Component({
  selector: 'aui-user-detail',
  templateUrl: './user-detail.component.html',
  styleUrls: ['./user-detail.component.less'],
  providers: [CapacityCalculateLabel]
})
export class UserDetailComponent implements OnInit {
  userId;
  node;
  spaceUsedRate;
  sizePercent = 0;
  usedSpaceQuota = 0;
  limitSpaceQuota = 0;
  unusedSpaceQuota = 0;
  unitconst = CAPACITY_UNIT;
  usedSizeColor = '#6C92FA';
  progressBarColor = [[0, '#6C92FA']];
  constructor(
    public sftpManagerApiService: SftpManagerApiService,
    public capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnInit() {
    this.getUserDetail();
  }

  getUserDetail() {
    this.sftpManagerApiService
      .querySftpUserDetailUsingGET({
        userId: this.userId,
        memberEsn: this.node
      })
      .subscribe(res => {
        if (!res) {
          return;
        }
        this.limitSpaceQuota = parseInt(res.limitSpaceQuota, 10);
        this.usedSpaceQuota = parseInt(res.usedSpaceQuota, 10);
        this.unusedSpaceQuota = this.limitSpaceQuota - this.usedSpaceQuota;
        this.sizePercent = parseFloat(res.spaceUsedRate);
        this.spaceUsedRate =
          this.capacityCalculateLabel.formatDecimalPoint(this.sizePercent, 3) +
          '%';
      });
  }
}
