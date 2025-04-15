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
import { DamengComponent } from './dameng.component';
import { DamengRoutingModule } from './dameng-routing.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SummaryModule } from './summary/summary.module';
import { RegisterModule } from './register/register.module';
import { CopyDataModule } from './copy-data/copy-data.module';

@NgModule({
  declarations: [DamengComponent],
  imports: [
    CommonModule,
    DamengRoutingModule,
    MultiClusterSwitchModule,
    BaseModule,
    ProButtonModule,
    ProTableModule,
    SummaryModule,
    RegisterModule,
    CopyDataModule
  ]
})
export class DamengModule {}
