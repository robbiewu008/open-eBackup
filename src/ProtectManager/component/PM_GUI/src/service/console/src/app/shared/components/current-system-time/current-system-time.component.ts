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
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { SystemTimeService } from 'app/shared/services/system-time.service';

@Component({
  selector: 'aui-current-system-time',
  templateUrl: './current-system-time.component.html',
  styleUrls: ['./current-system-time.component.less']
})
export class CurrentSystemTimeComponent implements OnInit, OnDestroy {
  @Input() cluster;
  @Input() needLoading: boolean;
  date;
  timeoutSysTime;
  sysTimeLong;

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private systemTimeService: SystemTimeService
  ) {}

  ngOnDestroy() {
    clearTimeout(this.timeoutSysTime);
  }

  ngOnInit() {
    this.getDate();
  }

  getAutoTime(displayName) {
    clearTimeout(this.timeoutSysTime);
    this.timeoutSysTime = setTimeout(() => {
      this.sysTimeLong += 1e3;
      this.date = this.i18n.get('common_current_device_time_label', [
        `${this.appUtilsService.convertDateLongToString(
          this.sysTimeLong
        )} ${displayName}`
      ]);
      this.getAutoTime(displayName);
    }, 1e3);
  }

  getDate() {
    this.systemTimeService
      .getSystemTime(this.needLoading ?? false, this.cluster)
      .subscribe(res => {
        this.date = this.i18n.get('common_current_device_time_label', [
          `${res.time} ${res.displayName}`
        ]);
        this.getAutoTime(res.displayName);
        this.sysTimeLong = new Date(res.time).getTime();
      });
  }
}
