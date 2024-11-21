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
import { DataMapService, I18NService } from '../services';

@Component({
  selector: 'sla-type',
  template: `
    <ng-container *ngIf="!!name; else elseTemplate">
      <ng-container [ngSwitch]="name">
        <ng-container *ngSwitchCase="'Gold'">
          <i lv-icon="aui-sla-gold" [lvColorState]="true"></i>
          {{ name }}
        </ng-container>
        <ng-container *ngSwitchCase="'Silver'">
          <i lv-icon="aui-sla-silver" [lvColorState]="true"></i>
          {{ name }}
        </ng-container>
        <ng-container *ngSwitchCase="'Bronze'">
          <i lv-icon="aui-sla-bronze" [lvColorState]="true"></i>
          {{ name }}
        </ng-container>
        <ng-container *ngSwitchDefault>
          <i lv-icon="aui-sla-myvmprotect" [lvColorState]="true"></i>
          {{ name }}
        </ng-container>
      </ng-container>
    </ng-container>
    <ng-template #elseTemplate>
      --
    </ng-template>
  `
})
export class SlaTypeComponent implements OnInit, OnChanges {
  @Input() name: string;

  constructor(
    public I18N: I18NService,
    public dataMapService: DataMapService
  ) {}

  ngOnInit() {}

  ngOnChanges(changes: SimpleChanges) {}
}

@NgModule({
  declarations: [SlaTypeComponent],
  imports: [CommonModule, IconModule],
  exports: [SlaTypeComponent]
})
export class SlaTypeModule {}
