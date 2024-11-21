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
import { IconModule } from '@iux/live';
import { toLower, toUpper } from 'lodash';
import { I18NService } from '../services';
import { AlarmColorConsts } from '../consts/common.const';

@Component({
  selector: 'alarm-level',
  template: `
    <ng-container [ngSwitch]="alarmType">
      <span class="alarm-icon-tag" [ngStyle]="{ background: color }">
        <i
          *ngSwitchCase="'info'"
          lv-icon="aui-icon-alarm-info-op"
          [lvColorState]="true"
        ></i>
        <i
          *ngSwitchCase="'major'"
          lv-icon="aui-icon-alarm-major-op"
          [lvColorState]="true"
        ></i>
        <i
          *ngSwitchCase="'warning'"
          lv-icon="aui-icon-alarm-warning-op"
          [lvColorState]="true"
        ></i>
        <i
          *ngSwitchCase="'critical'"
          lv-icon="aui-icon-alarm-critical-op"
          [lvColorState]="true"
        ></i>
        <span class="alarm-label">{{ alarmLabel || '--' }}</span>
      </span>
    </ng-container>
  `,
  styles: [
    `
      .alarm-icon-tag {
        display: inline-block;
        min-width: 60px;
        height: 24px;
        color: #ffffff;
        border-radius: 14px;
        line-height: 23px;
        padding-right: 8px;
      }
      .alarm-icon-tag i {
        margin-left: 4px;
        margin-top: -2px;
        cursor: default;
      }
      .alarm-label {
        margin-left: 4px;
      }
    `
  ]
})
export class AlarmLevelComponent implements OnInit, OnChanges {
  @Input() type: string;
  alarmLabel: string;
  alarmType;
  color: string;

  constructor(public i18n: I18NService) {}

  ngOnInit() {
    this.init();
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.type) {
      this.init();
    }
  }

  init() {
    this.alarmType = toLower(this.type);
    this.alarmLabel = this.i18n.get(`common_alarms_${this.alarmType}_label`);
    this.color = AlarmColorConsts[toUpper(this.type)];
  }
}

@NgModule({
  declarations: [AlarmLevelComponent],
  imports: [CommonModule, IconModule],
  exports: [AlarmLevelComponent]
})
export class AlarmLevelModule {}
