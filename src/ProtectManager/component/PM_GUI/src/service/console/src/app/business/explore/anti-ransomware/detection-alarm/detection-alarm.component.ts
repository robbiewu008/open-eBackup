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
import { ChangeDetectorRef, Component, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import {
  ALARM_NAVIGATE_STATUS,
  CommonConsts,
  ConfigManagementService,
  I18NService
} from 'app/shared';

@Component({
  selector: 'aui-detection-alarm',
  templateUrl: './detection-alarm.component.html',
  styleUrls: ['./detection-alarm.component.less']
})
export class DetectionAlarmComponent implements OnInit {
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  alarmData = [];

  constructor(
    public router: Router,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private configManagementService: ConfigManagementService
  ) {}

  ngOnInit() {
    this.getAntiAlarms();
  }

  getDetail(data, isSan = false) {
    ALARM_NAVIGATE_STATUS.location = data.vstoreName;
    ALARM_NAVIGATE_STATUS.alarmId = isSan ? '0x5F025D0015' : '0x5F025D0004';
    this.router.navigate(['insight/alarms']);
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getAntiAlarms();
  }

  getAntiAlarms() {
    const params = {
      pageSize: this.pageSize,
      pageNum: this.pageIndex + 1
    };
    this.configManagementService.getIoDetectSummary(params).subscribe(res => {
      this.alarmData = res.records;
      this.total = res.totalCount;
    });
  }
}
