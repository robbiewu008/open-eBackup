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
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import {
  CheckboxModule,
  GroupModule,
  IconModule,
  PopoverModule
} from '@iux/live';
import { ProColsDisplayComponent } from './pro-cols-display.component';

@NgModule({
  declarations: [ProColsDisplayComponent],
  imports: [
    CommonModule,
    FormsModule,
    PopoverModule,
    IconModule,
    GroupModule,
    CheckboxModule
  ],
  exports: [ProColsDisplayComponent]
})
export class ProColsDisplayModule {}
