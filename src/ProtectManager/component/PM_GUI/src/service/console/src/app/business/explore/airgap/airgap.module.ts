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
import { AirgapRoutingModule } from './airgap-routing.modules';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { AirgapComponent } from './airgap.component';
import { AirgapTacticsModule } from '../policy/airgap/airgap-tactics.module';
import { StorageDeviceComponent } from './storage-device/storage-device.component';
import { StorageDeviceModule } from './storage-device/storage-device.module';

@NgModule({
  declarations: [AirgapComponent],
  imports: [
    CommonModule,
    BaseModule,
    AirgapRoutingModule,
    MultiClusterSwitchModule,
    AirgapTacticsModule,
    StorageDeviceModule
  ]
})
export class AirgapModule {}
