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
import { FusionComputeComponent } from './fusion-compute.component';
import { FusionComputeRoutingModule } from './fusion-compute-routing.module';
import { LiveMountsListModule } from 'app/business/explore/live-mounts/live-mounts-list/live-mounts-list.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { FusionListModule } from './fusion-list/fusion-list.module';
import { RegisterFusionComputeModule } from './register-fusion-compute/register-fusion-compute.module';
import { EnvironmentInfoModule } from './fusion-list/environment-info/environment-info.module';
import { VirtualizationGroupModule } from '../virtualization-group/virtualization-group.module';

@NgModule({
  declarations: [FusionComputeComponent],
  imports: [
    CommonModule,
    FusionComputeRoutingModule,
    BaseModule,
    LiveMountsListModule,
    MultiClusterSwitchModule,
    RegisterFusionComputeModule,
    FusionListModule,
    EnvironmentInfoModule,
    VirtualizationGroupModule
  ],
  exports: [FusionComputeComponent]
})
export class FusionComputeModule {}
