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
import { Component, NgModule, OnInit } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CheckboxModule } from '@iux/live';
import { I18NService } from 'app/shared/services/i18n.service';

@Component({
  selector: 'aui-backup',
  template: `
    <div class="backup-content">
      <span [innerHTML]="content"></span>
    </div>

    <div class="backup-checkbox">
      <label lv-checkbox [(ngModel)]="status">
        {{ i18n.get('protection_sla_perform_backup_now_label') }}
      </label>
    </div>
  `,
  styles: [
    `
      .backup-content {
        max-height: 240px;
        overflow: auto;
      }

      .backup-checkbox {
        margin-top: 24px;
      }
    `
  ]
})
export class BackupComponent implements OnInit {
  status = true;
  content = this.i18n.get('protection_protect_late_tip_label');

  constructor(public i18n: I18NService) {}

  ngOnInit() {}
}

@NgModule({
  imports: [FormsModule, CheckboxModule],
  declarations: [BackupComponent],

  exports: [BackupComponent]
})
export class BackupModule {}
