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
import { CommonModule } from '@angular/common';
import { Component, Input, NgModule, OnInit } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { IconModule, TooltipModule } from '@iux/live';
import { I18NService } from 'app/shared/services/i18n.service';
import { RouterUrl } from '../consts/permission.const';
import { AppUtilsService } from '../services/app-utils.service';

@Component({
  selector: 'aui-agent-jumper-tips',
  template: `
    <i
      lv-icon="aui-icon-help"
      [lv-tooltip]="agentTipsTpl"
      lvTooltipTheme="light"
      class="configform-constraint"
      lvColorState="true"
      (mouseenter)="helpHover()"
    ></i>

    <ng-template #agentTipsTpl>
      <span [innerHTML]="agentTips"></span>
    </ng-template>
  `
})
export class AgentsJumperTipsComponent implements OnInit {
  agentTips;

  @Input() type;
  constructor(
    public i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.agentTips = this.i18n.get('protection_agents_link_tips_label', [
      this.type
    ]);
  }

  helpHover() {
    this.appUtilsService.openRouter(RouterUrl.ProtectionHostAppHost);
  }
}

@NgModule({
  imports: [CommonModule, FormsModule, TooltipModule, IconModule],
  declarations: [AgentsJumperTipsComponent],

  exports: [AgentsJumperTipsComponent]
})
export class AgentsJumperTipsModule {}
