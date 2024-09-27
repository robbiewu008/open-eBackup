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
import { DataMap, I18NService } from 'app/shared';
import { isUndefined } from 'lodash';

@Component({
  selector: 'aui-anti-policy-setting',
  templateUrl: './anti-policy-setting.component.html',
  styleUrls: ['./anti-policy-setting.component.less']
})
export class AntiPolicySettingComponent implements OnInit {
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  activeIndex = 'policy';
  constructor(private i18n: I18NService) {}

  ngOnInit() {
    if (!isUndefined(localStorage.getItem('setCopyLimit'))) {
      // 从保护那里打开新窗口过来可以直接跳到感染副本操作限制添加界面
      if (localStorage.getItem('setCopyLimit') === '0') {
        this.activeIndex = 'limit';
      }
      localStorage.removeItem('setCopyLimit');
    }
  }
}
