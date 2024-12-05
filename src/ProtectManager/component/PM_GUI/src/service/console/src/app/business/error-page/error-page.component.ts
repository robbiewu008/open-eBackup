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
  BaseUtilService,
  CookieService,
  DataMap,
  GROUP_COMMON,
  I18NService,
  LANGUAGE,
  UsersApiService
} from 'app/shared';
import { includes } from 'lodash';

@Component({
  selector: 'aui-error-page',
  templateUrl: './error-page.component.html',
  styleUrls: ['./error-page.component.less']
})
export class ErrorPageComponent implements OnInit {
  groupOptions = GROUP_COMMON;
  userId;
  isSend = false;
  languageLabel =
    this.i18n.language.toLowerCase() === LANGUAGE.CN
      ? this.i18n.get('common_english_label')
      : this.i18n.get('common_chinese_label');
  isCloudBackup = includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value
    ],
    this.i18n.get('deploy_type')
  );
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  countdownLabel = this.i18n.get('common_countdown_label', [900]);
  errorTitle;
  errorDesc;
  constructor(
    private route: Router,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public usersApiService: UsersApiService
  ) {}

  ngOnInit() {
    this.errorTitle = includes(this.route.url, 'type=inner')
      ? this.i18n.get('system_no_permission_label')
      : '';
    this.errorDesc = includes(this.route.url, 'type=inner')
      ? this.i18n.get('system_no_permission_desc_label')
      : this.i18n.get('system_limit_desc_label');
  }

  toggleLanguage() {
    this.i18n.changeLanguage(
      this.i18n.language.toLowerCase() === LANGUAGE.CN
        ? LANGUAGE.EN
        : LANGUAGE.CN
    );
  }
}
