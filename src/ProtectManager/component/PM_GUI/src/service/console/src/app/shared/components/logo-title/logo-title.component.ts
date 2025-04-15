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
import { Component, Input, OnInit } from '@angular/core';
import {
  WhiteboxService,
  IMAGE_PATH_PREFIX
} from '../../services/whitebox.service';
import { I18NService } from 'app/shared/services/i18n.service';
import { BaseUtilService, DataMap, ThemeEnum, getAppTheme } from 'app/shared';
import { includes } from 'lodash';

@Component({
  selector: 'aui-logo-title',
  templateUrl: './logo-title.component.html',
  styleUrls: ['./logo-title.component.less']
})
export class LogoTitleComponent implements OnInit {
  @Input() isWhiteColor: boolean;
  @Input() isMenu: boolean;
  @Input() tinySizeFont: boolean;
  @Input() isResetPwd: boolean;
  @Input() cyberDarkHeader: boolean;

  readonly title = this.baseUtil.getProductName();
  readonly isWhitebox = this.whitebox.isWhitebox;

  constructor(
    public i18n: I18NService,
    private whitebox: WhiteboxService,
    private baseUtil: BaseUtilService
  ) {}

  ngOnInit() {}

  isBlackImage() {
    // 安全一体机首页
    if (this.cyberDarkHeader) {
      return false;
    }
    // 找回密码页面
    if (this.isResetPwd) {
      return true;
    }
    // 登录页和关于页面
    if (this.isWhiteColor) {
      return false;
    }
    // 其它形态默认浅模式
    if (
      includes(
        [
          DataMap.Deploy_Type.cloudbackup.value,
          DataMap.Deploy_Type.cloudbackup2.value,
          DataMap.Deploy_Type.hyperdetect.value,
          DataMap.Deploy_Type.cyberengine.value
        ],
        this.i18n.get('deploy_type')
      )
    ) {
      return true;
    }
    // X系列、E系列随深浅模式切换
    return getAppTheme(this.i18n) === ThemeEnum.light;
  }

  initLogoImage() {
    const isWhitebox = this.whitebox.isWhitebox;
    const isOpenBackup = includes(
      [DataMap.Deploy_Type.openOem.value, DataMap.Deploy_Type.openServer.value],
      this.i18n.get('deploy_type')
    );
    return {
      'background-image': `url(${
        isWhitebox
          ? IMAGE_PATH_PREFIX + 'logo.png'
          : isOpenBackup
          ? 'assets/img/open-ebackup-logo.svg'
          : this.isBlackImage()
          ? 'assets/img/huawei_logo_black.svg'
          : 'assets/img/huawei_logo_light.svg'
      })`,
      'background-repeat': 'no-repeat',
      'background-size': 'cover'
    };
  }
}
