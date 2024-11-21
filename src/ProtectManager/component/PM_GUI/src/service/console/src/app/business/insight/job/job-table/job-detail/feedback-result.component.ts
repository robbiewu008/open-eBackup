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
import { CheckboxModule, GroupModule, RadioModule } from '@iux/live';
import { I18NService } from 'app/shared/services/i18n.service';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-backup',
  template: `
    <div class="backup-content">
      <span>{{ i18n.get('insight_feedback_restore_result_tips_label') }}</span>
    </div>

    <div class="aui-gutter-column-lg"></div>
    <div class="backup-checkbox">
      <lv-radio-group [(ngModel)]="result" [lvGroupName]="'group'">
        <lv-group [lvGutter]="'16px'">
          <lv-radio [lvValue]="'SUCCESS'">{{
            i18n.get('insight_restore_successed_label')
          }}</lv-radio>
          <lv-radio [lvValue]="'FAIL'">{{
            i18n.get('common_status_restore_failed_label')
          }}</lv-radio>
        </lv-group>
      </lv-radio-group>
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
export class FeedbackResultComponent implements OnInit {
  status = true;
  content = this.i18n.get('protection_protect_late_tip_label');
  result = 'SUCCESS';
  valid$ = new Subject<boolean>();

  constructor(public i18n: I18NService) {}

  ngOnInit() {}
}

@NgModule({
  imports: [FormsModule, RadioModule, GroupModule, CommonModule],
  declarations: [FeedbackResultComponent],
  exports: [FeedbackResultComponent]
})
export class FeedbackResultModule {}
