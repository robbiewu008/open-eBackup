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
import { BaseUtilService, DataMap } from 'app/shared';
import { each, first, includes, set } from 'lodash';

@Component({
  selector: 'aui-logo-title',
  templateUrl: './logo-title.component.html',
  styleUrls: ['./logo-title.component.less']
})
export class LogoTitleComponent implements OnInit {
  @Input() isMenu: boolean;
  @Input() colorDark: boolean;

  readonly title = this.baseUtil.getProductName();
  readonly isWhitebox = this.whitebox.isWhitebox;

  constructor(
    public i18n: I18NService,
    private whitebox: WhiteboxService,
    private baseUtil: BaseUtilService
  ) {}

  ngOnInit() {
    setTimeout(() => {
      this.initLogoImage();
    });
  }

  isBlackImage(logo) {
    return this.isMenu && logo && includes(logo.className, 'black_logo');
  }

  initLogoImage() {
    const isWhitebox = this.whitebox.isWhitebox;
    const isOpenBackup = includes(
      [DataMap.Deploy_Type.openOem.value, DataMap.Deploy_Type.openServer.value],
      this.i18n.get('deploy_type')
    );
    const containers = document.getElementsByClassName('logo_title_container');
    each(containers, container => {
      const logo = first(container.getElementsByClassName('logo'));
      set(
        logo,
        'style',
        `background:url(${
          isWhitebox
            ? IMAGE_PATH_PREFIX + 'logo.png'
            : isOpenBackup
            ? 'assets/img/open-ebackup-logo.svg'
            : this.isBlackImage(logo)
            ? 'assets/img/huaweilogo-black.svg'
            : 'assets/img/huaweilogo.svg'
        }) no-repeat;background-size:cover;`
      );
    });
  }
}
