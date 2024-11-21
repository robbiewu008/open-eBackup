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
import { get } from 'lodash';
import {
  AlarmAndEventApiService,
  DataMap,
  I18NService,
  RouterUrl
} from 'app/shared';
import { Router } from '@angular/router';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'app-alarms-count',
  templateUrl: './alarms-count.component.html',
  styleUrls: ['./alarms-count.component.less']
})
export class AlarmsCountComponent implements OnInit {
  criticalCount: number = 0;
  majorCount: number = 0;
  warningCount: number = 0;
  totalCount: number = 0;
  alarmSeverity = DataMap.Alarm_Severity_Type;
  constructor(
    private alarmAndEventApiService: AlarmAndEventApiService,
    public i18n: I18NService,
    private router: Router,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.loadData();
  }
  loadData(loading: boolean = true) {
    const params: { [key: string]: any } = {
      pageSize: 99,
      pageNum: 0
    };
    this.alarmAndEventApiService
      .queryAlarmCountBySeverityUsingGET({
        akLoading: false,
        akDoException: false
      })
      .subscribe(res => {
        this.majorCount = res.major;
        this.criticalCount = res.critical;
        this.warningCount = res.warning;
        this.totalCount =
          this.criticalCount + this.majorCount + this.warningCount;
      });
  }

  widthComputer(key: string) {
    return `${((get(this, key, 0) as any) / this.totalCount) * 100}%`;
  }

  gotoAlarm($event, severity?) {
    if ($event?.stopPropagation) {
      $event?.stopPropagation();
    }
    if (severity) {
      this.appUtilsService.setCacheValue('alarmSeverity', severity);
    }
    this.router.navigateByUrl(RouterUrl.InsightAlarms);
  }
}
