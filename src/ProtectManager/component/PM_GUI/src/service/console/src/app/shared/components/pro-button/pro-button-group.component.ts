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
import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnChanges,
  OnInit,
  SimpleChanges,
  ViewEncapsulation
} from '@angular/core';
import { I18NService } from 'app/shared';
import { ProButton } from './interface';
import { isUndefined } from 'lodash';

@Component({
  selector: 'lv-pro-button-group',
  templateUrl: './pro-button.component.html',
  styleUrls: ['./pro-button.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  encapsulation: ViewEncapsulation.None
})
export class ProButtonGroupComponent implements OnInit, OnChanges {
  @Input() config: ProButton[];
  @Input() bindData: any[]; // 按钮操作的对象
  @Input() mode: 'link' | 'button' = 'button';
  @Input() maxDisplayItems = 5;
  @Input() keepDropdown = true;
  @Input() menuText: string = this.i18n.get('common_more_label');

  totalButtons: any = [];
  buttons: any = [];
  buttonMore: ProButton;
  isGroup = true;

  constructor(private cdr: ChangeDetectorRef, private i18n: I18NService) {}

  ngOnInit() {
    this._checkButtonStatus();
  }

  ngOnChanges(changes: SimpleChanges): void {
    if (changes.bindData) {
      this._checkButtonStatus();
    }
  }

  /**
   * 根据外部条件变化去触发按钮状态的更新
   */
  _checkButtonStatus() {
    this.keepDropdown = isUndefined(this.keepDropdown)
      ? true
      : this.keepDropdown;
    this.totalButtons = this.config.filter(item => {
      const _data = this.bindData || [];
      return item.displayCheck ? item.displayCheck(_data) : true;
    });
    if (
      this.totalButtons.length >
      (this.keepDropdown ? this.maxDisplayItems - 1 : this.maxDisplayItems)
    ) {
      this.buttons = this.totalButtons.filter(
        (item, index) => index < this.maxDisplayItems - 1
      );
      this.buttonMore = {
        label: this.menuText,
        items: this.totalButtons.filter(
          (item, index) => index >= this.maxDisplayItems - 1
        )
      };
    } else {
      this.buttons = this.totalButtons.filter(
        (item, index) => index < this.maxDisplayItems
      );
    }
    this.cdr.markForCheck();
  }
}
