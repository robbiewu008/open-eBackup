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
import { I18NService } from 'app/shared/services/i18n.service';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-backup',
  template: `
    <div class="backup-content">
      <span [innerHTML]="content"></span>
    </div>
    <div class="backup-content">
      <span>{{ i18n.get('protection_protect_mysql_tip_label') }}</span>
    </div>

    <div class="aui-gutter-column-lg"></div>
    <div class="backup-checkbox" *ngIf="askManualBackup">
      <label lv-checkbox [(ngModel)]="status">
        {{ i18n.get('protection_sla_perform_backup_now_label') }}
      </label>
    </div>
    <div class="confirm-checkbox">
      <label
        lv-checkbox
        [(ngModel)]="confirm"
        (ngModelChange)="onChange($event)"
      >
        {{ i18n.get('common_warning_confirm_label') }}
      </label>
    </div>
  `,
  styles: [
    `
      .backup-content {
        word-break: break-all;
        max-height: 240px;
        overflow: auto;
      }

      .backup-checkbox {
        margin-top: 12px;
      }

      .confirm-checkbox {
        margin-top: 12px;
        word-break: break-all;
      }
    `
  ]
})
export class MySQLBackupComponent implements OnInit {
  status = true;
  confirm = false;
  content = this.i18n.get('protection_protect_late_tip_label');
  askManualBackup = false;
  manualBackup = false;
  valid$ = new Subject<boolean>();

  constructor(public i18n: I18NService) {}

  ngOnInit() {
    this.content = this.manualBackup
      ? this.i18n.get('protection_manual_backup_protect_mysql_tip_label')
      : this.askManualBackup
      ? this.i18n.get('protection_protect_late_tip_label')
      : this.i18n.get('protection_protect_mysql_simple_tip_label');
  }

  onChange($event) {
    this.valid$.next(this.confirm);
  }
}

@NgModule({
  imports: [FormsModule, CheckboxModule, CommonModule],
  declarations: [MySQLBackupComponent],

  exports: [MySQLBackupComponent]
})
export class BackupModule {}
