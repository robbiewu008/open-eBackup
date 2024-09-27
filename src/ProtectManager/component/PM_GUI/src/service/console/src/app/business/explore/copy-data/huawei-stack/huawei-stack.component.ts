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
  ResourceType,
  DataMap,
  I18NService,
  CookieService,
  CommonConsts
} from 'app/shared';

@Component({
  selector: 'aui-huawei-stack',
  templateUrl: './huawei-stack.component.html',
  styleUrls: ['./huawei-stack.component.less']
})
export class HuaweiStackComponent implements OnInit {
  header =
    this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE
      ? this.i18n.get('common_cloud_server_label')
      : this.i18n.get('common_cloud_label');
  resourceType = ResourceType.HCS;
  childResourceType = [DataMap.Resource_Type.HCSCloudHost.value];

  constructor(
    private i18n: I18NService,
    private cookieService: CookieService
  ) {}

  ngOnInit(): void {}
}
