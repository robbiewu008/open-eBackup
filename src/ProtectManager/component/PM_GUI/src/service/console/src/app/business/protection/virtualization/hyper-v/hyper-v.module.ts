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

import { VirtualizationBaseModule } from '../virtualization-base/virtualization-base.module';
import { HyperVRoutingModule } from './hyper-v-routing.module';
import { HyperVComponent } from './hyper-v.component';
import { HypervCopyDataModule } from './hyperv-copy-data/hyperv-copy-data.module';
import { SummaryModule } from './summary/summary.module';

@NgModule({
  declarations: [HyperVComponent],
  imports: [
    CommonModule,
    HyperVRoutingModule,
    VirtualizationBaseModule,
    SummaryModule,
    HypervCopyDataModule
  ],
  exports: [HyperVComponent]
})
export class HyperVModule {}
