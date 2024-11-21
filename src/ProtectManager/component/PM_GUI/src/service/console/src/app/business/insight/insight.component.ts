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
  I18NService,
  CookieService,
  getAccessibleMenu,
  GlobalService,
  DataMap,
  CommonConsts
} from 'app/shared';
import { includes } from 'lodash';

@Component({
  selector: 'insight',
  templateUrl: './insight.component.html'
})
export class InsightComponent implements OnInit {
  menus = [];
  collapsed = false;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  constructor(
    public i18n: I18NService,
    public cookieService: CookieService,
    public globalService: GlobalService
  ) {}

  ngOnInit() {
    this.globalService.getUserInfo().subscribe(res => {
      this.initMenus();
    });
  }

  initMenus() {
    const menus = [
      {
        id: 'performance',
        label: this.i18n.get('common_performance_label'),
        icon: 'aui-icon-performance',
        routerLink: '/insight/performance',
        hidden:
          this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
      },
      {
        id: 'alarms',
        label: this.i18n.get('common_alarms_events_label'),
        icon: 'aui-icon-alarms',
        routerLink: '/insight/alarms'
      },
      {
        id: 'jobs',
        label: this.i18n.get('common_jobs_label'),
        icon: 'aui-icon-jobs',
        routerLink: '/insight/jobs'
      },
      {
        id: 'reports',
        label: this.i18n.get('common_report_label'),
        icon: 'aui-icon-reports',
        routerLink: '/insight/reports',
        hidden: !includes(
          [
            DataMap.Deploy_Type.x3000.value,
            DataMap.Deploy_Type.x6000.value,
            DataMap.Deploy_Type.x8000.value,
            DataMap.Deploy_Type.a8000.value,
            DataMap.Deploy_Type.x9000.value,
            DataMap.Deploy_Type.e6000.value,
            DataMap.Deploy_Type.decouple.value,
            DataMap.Deploy_Type.openServer.value
          ],
          this.i18n.get('deploy_type')
        )
      }
    ];
    this.menus = getAccessibleMenu(menus, this.cookieService, this.i18n);
  }
}
