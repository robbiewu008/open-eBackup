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
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import {
  ButtonModule,
  CheckboxModule,
  DatatableModule,
  GroupModule,
  IconModule,
  InputModule,
  LoadingModule,
  OverflowModule,
  PaginatorModule,
  PopoverModule,
  RadioModule,
  TagModule,
  TooltipModule
} from '@iux/live';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from '../pro-button/index';
import { ProProgressModule } from '../pro-progress/index';
import { ProStatusModule } from '../pro-status/index';
import { ProTextModule } from '../pro-text/index';
import { ProColsDisplayModule } from './pro-cols-display/index';
import { ProFilterDateModule } from './pro-filter-date/index';
import { ProFilterSearchModule } from './pro-filter-search/index';
import { ProTableComponent } from './pro-table.component';
import { CustomTableSearchModule } from '../custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [ProTableComponent],
  imports: [
    CommonModule,
    FormsModule,
    DatatableModule,
    ButtonModule,
    GroupModule,
    IconModule,
    PaginatorModule,
    InputModule,
    TagModule,
    LoadingModule,
    ProFilterSearchModule,
    ProFilterDateModule,
    ProColsDisplayModule,
    PopoverModule,
    TooltipModule,
    OverflowModule,
    CheckboxModule,
    RadioModule,
    ProProgressModule,
    ProStatusModule,
    ProTextModule,
    ProButtonModule,
    BaseModule,
    CustomTableSearchModule
  ],
  exports: [ProTableComponent]
})
export class ProTableModule {}
