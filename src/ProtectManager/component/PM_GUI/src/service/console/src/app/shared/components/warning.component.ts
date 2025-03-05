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
import { Component, NgModule, OnInit, TemplateRef } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { CheckboxModule } from '@iux/live';
import { DataMap, OperateItems } from 'app/shared/consts';
import { I18NService } from 'app/shared/services/i18n.service';
import { every, isArray, size } from 'lodash';
import { Subject } from 'rxjs';
import { NgIf, NgTemplateOutlet } from '@angular/common';

@Component({
  selector: 'aui-warning',
  template: `
    <div class="warning-content">
      <span *ngIf="contentIsString" [innerHTML]="content"></span>
      <ng-container *ngIf="contentIsTemplateRef">
        <ng-template *ngTemplateOutlet="content"></ng-template>
      </ng-container>
    </div>
    <div
      *ngIf="showForciblyDeleteCopy"
      class="warning-checkbox force-delete-checkbox"
    >
      <label lv-checkbox [(ngModel)]="forciblyDeleteCopy">{{
        i18n.get('common_forcibly_delete_copy_label')
      }}</label>
    </div>
    <div
      *ngIf="showForciblyDelete"
      class="warning-checkbox force-delete-checkbox"
    >
      <label lv-checkbox [(ngModel)]="forciblyDelete">{{
        i18n.get('common_force_delete_label')
      }}</label>
      <div>{{ i18n.get('common_force_delete_tip_label') }}</div>
    </div>
    <div class="warning-checkbox">
      <label
        lv-checkbox
        [(ngModel)]="status"
        (ngModelChange)="warningConfirmChange($event)"
        >{{ i18n.get('common_warning_confirm_label') }}</label
      >
    </div>
  `,
  styles: [
    `
      .warning-content {
        max-height: 240px;
        overflow: auto;
      }

      .force-delete-checkbox {
        margin-top: 8px;
        margin-bottom: -16px;
      }
    `
  ]
})
export class WarningComponent implements OnInit {
  status;
  content;
  rowData;
  actionId;
  isChecked$ = new Subject<boolean>();
  forciblyDeleteCopy = null; // 默认为null，接口判断为null时不会下发该字段
  forciblyDelete = null;
  showForciblyDeleteCopy = false;
  showForciblyDelete = false;
  contentIsString = false;
  contentIsTemplateRef = false;

  constructor(public i18n: I18NService) {}

  ngOnInit() {
    this.contentIsString = typeof this.content === 'string';
    this.contentIsTemplateRef = this.content instanceof TemplateRef;
    this.showForciblyDeleteCopy = this.checkDeleteCopyOpts();
    this.showForciblyDelete = this.checkForceDelete();
  }

  warningConfirmChange(e) {
    this.isChecked$.next(e);
  }

  /**
   * 判断是否满足删除副本的条件
   * 1、操作为'删除副本'
   * 2、副本状态为'删除失败'
   * 3、对单个副本进行操作
   * 满足以上条件才会展示强制删除副本选项
   */
  checkDeleteCopyOpts(): boolean {
    if (this.actionId !== OperateItems.DeletingCopy) {
      return false;
    }
    // 数组
    if (isArray(this.rowData)) {
      return every(
        this.rowData,
        item => item.status === DataMap.copydata_validStatus.deleteFailed.value
      );
    }
    // 对象
    return (
      this.rowData?.status === DataMap.copydata_validStatus.deleteFailed.value
    );
  }

  /**
   * 判断是否满足强制删除备份存储单元的条件
   * 1、操作为'删除备份存储单元'
   * 2、备份存储单元运行状态为'离线'
   * 满足以上条件才会展示强制删除选项
   */
  checkForceDelete(): boolean {
    if (this.actionId !== OperateItems.DeleteBackupStorageUnit) {
      return false;
    }
    // 数组
    if (isArray(this.rowData)) {
      return every(
        this.rowData,
        item =>
          item?.runningStatus === DataMap.StoragePoolRunningStatus.offline.value
      );
    }
    // 对象
    return (
      this.rowData?.runningStatus ===
      DataMap.StoragePoolRunningStatus.offline.value
    );
  }
}

@NgModule({
  imports: [FormsModule, CheckboxModule, NgIf, NgTemplateOutlet],
  declarations: [WarningComponent],

  exports: [WarningComponent]
})
export class WarningModule {}
