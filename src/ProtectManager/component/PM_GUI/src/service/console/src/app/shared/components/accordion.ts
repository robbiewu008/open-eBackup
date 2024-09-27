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
import { Component, Output, NgModule, EventEmitter } from '@angular/core';
import { IconModule } from '@iux/live';
import { I18NService } from '../services';

@Component({
  selector: 'aui-accordion',
  template: `
    <h2 (click)="click()" class="title-lg">
      {{ I18N.get('common_advanced_label') }}
      <i
        [ngClass]="show && 'down'"
        lv-icon="lv-icon-triangle-down"
        [lvColorState]="true"
      ></i>
    </h2>
  `,
  styles: [
    `
      .title-lg {
        cursor: pointer;
      }
      .title-lg:hover {
        color: #5d7ede;
      }
      .title-lg:hover i {
        color: #5d7ede;
      }
      .down {
        transform: scaleY(-1) translateY(1px);
        transition: transform 0.2s linear;
      }
    `
  ]
})
export class AccordionComponent {
  show = false;
  @Output() accordionClick = new EventEmitter<any>();

  constructor(public I18N: I18NService) {}

  click() {
    this.show = !this.show;
    this.accordionClick.emit();
  }
}

@NgModule({
  declarations: [AccordionComponent],
  imports: [CommonModule, IconModule],
  exports: [AccordionComponent]
})
export class AccordionModule {}
