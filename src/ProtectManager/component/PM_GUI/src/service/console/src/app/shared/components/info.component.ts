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
import { Component, NgModule, OnInit } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CheckboxModule } from '@iux/live';
import { I18NService } from '../services/i18n.service';

@Component({
  selector: 'aui-info',
  template: `
    <div class="info-content">
      <ng-container *ngIf="noBreak; else elseTemplate">
        <span>{{ content }}</span>
      </ng-container>
      <ng-template #elseTemplate>
        <span [innerHTML]="content"></span>
      </ng-template>
    </div>
  `,
  styles: [
    `
      .info-content {
        word-break: break-all;
        max-height: 240px;
        overflow: auto;
      }
    `
  ]
})
export class InfoComponent implements OnInit {
  content;
  noBreak;

  constructor(public i18n: I18NService) {}

  ngOnInit() {}
}

@NgModule({
  imports: [CommonModule, FormsModule, CheckboxModule],
  declarations: [InfoComponent],

  exports: [InfoComponent]
})
export class InfoModule {}
