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
import { NavigationEnd, Router } from '@angular/router';
import { I18NService, RouterUrl } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-cloud-app',
  templateUrl: './cloud.component.html',
  styleUrls: ['./cloud.component.less']
})
export class CloudComponent implements OnInit {
  subApp = [...this.appUtilsService.getApplicationConfig().cloud];
  typeTitle = this.i18n.get('common_huawei_clouds_label');
  isPrivateCloud = this.router.url === RouterUrl.ProtectionHcsCloudHuaweiStack;

  constructor(
    public router: Router,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {
    this.routeChange();
  }

  routeChange() {
    this.router.events.subscribe(event => {
      if (event instanceof NavigationEnd) {
        this.isPrivateCloud =
          this.router.url === RouterUrl.ProtectionHcsCloudHuaweiStack;
      }
    });
  }
}
