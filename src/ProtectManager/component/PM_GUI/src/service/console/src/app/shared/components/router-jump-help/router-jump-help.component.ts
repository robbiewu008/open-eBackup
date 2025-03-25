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
import { Component, Input } from '@angular/core';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-router-jump-help',
  templateUrl: './router-jump-help.component.html',
  styleUrls: ['./router-jump-help.component.less']
})
export class RouterJumpHelpComponent {
  @Input() routerUrl: string;
  @Input() helpTips: string;
  @Input() tipPosition = 'right';

  constructor(private appUtilsService: AppUtilsService) {}

  helpHover() {
    this.appUtilsService.openRouter(this.routerUrl);
  }
}
