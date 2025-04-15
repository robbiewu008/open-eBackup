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
import {
  Component,
  Input,
  NgModule,
  OnChanges,
  SimpleChanges,
  OnInit
} from '@angular/core';
import { OverflowModule } from '@iux/live';
import { ColorConsts } from '../consts';
import { DataMapService, I18NService } from '../services';

@Component({
  selector: 'aui-status',
  template: `
    <div class="status" lv-overflow>
      <ng-container *ngIf="label !== '--'; else elseTemplate">
        <i [ngStyle]="{ background: color }"></i>
        <span>{{ label }}</span>
      </ng-container>
      <ng-template #elseTemplate>
        --
      </ng-template>
    </div>
  `,
  styles: [
    `
      .status > i {
        height: 10px;
        width: 10px;
        display: inline-block;
        border-radius: 50%;
        margin-right: 8px;
      }
    `
  ]
})
export class StatusComponent implements OnInit, OnChanges {
  @Input() type: string;

  @Input() value;

  label;

  color;

  constructor(
    public I18N: I18NService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.init();
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.type || changes.value) {
      this.init();
    }
  }

  init() {
    const config =
      this.dataMapService.getValueConfig(this.type, this.value, false) || {};
    this.color = config.color || ColorConsts.RUNNING;
    this.label = this.I18N.get(config.label || '--');
  }
}

@NgModule({
  declarations: [StatusComponent],
  imports: [CommonModule, OverflowModule],
  exports: [StatusComponent]
})
export class StatusModule {}
