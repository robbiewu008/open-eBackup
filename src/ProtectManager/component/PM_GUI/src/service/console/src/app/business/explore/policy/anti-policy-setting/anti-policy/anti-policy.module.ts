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
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AntiPolicyDetailModule } from './anti-policy-detail/anti-policy-detail.module';
import { AntiPolicyComponent } from './anti-policy.component';
import { CreateAntiPolicyModule } from './create-anti-policy/create-anti-policy.module';

@NgModule({
  declarations: [AntiPolicyComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    CreateAntiPolicyModule,
    AntiPolicyDetailModule,
    MultiClusterSwitchModule
  ],
  exports: [AntiPolicyComponent]
})
export class AntiPolicyModule {}
