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
import { HuaWeiStackListModule } from './stack-list/huawei-stack-list.module';
import { HuaweiStackRoutingModule } from './huawei-stack-routing.module';
import { HuaweiStackComponent } from './huawei-stack.component';
import { RegisterHuaWeiStackModule } from './register-huawei-stack/register-huawei-stack.module';
import { EnvironmentInfoModule } from './stack-list/environment-info/environment-info.module';
import { CloudServerModule } from './cloud-server/cloud-server.module';
import { VirtualizationGroupModule } from '../../virtualization/virtualization-group/virtualization-group.module';

@NgModule({
  declarations: [HuaweiStackComponent],
  imports: [
    CommonModule,
    HuaweiStackRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    HuaWeiStackListModule,
    RegisterHuaWeiStackModule,
    EnvironmentInfoModule,
    CloudServerModule,
    VirtualizationGroupModule
  ]
})
export class HuaweiStackModule {}
