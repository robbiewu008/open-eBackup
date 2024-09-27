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
import { BaseModule } from 'app/shared';
import { StorageDeviceComponent } from './storage-device.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { DeviceDetailModule } from './device-detail/device-detail.module';
import { GetTacticsComponent } from './get-tactics/get-tactics.component';
import { GetTacticsModule } from './get-tactics/get-tactics.module';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [StorageDeviceComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    GetTacticsModule,
    DeviceDetailModule
  ],
  exports: [StorageDeviceComponent]
})
export class StorageDeviceModule {}
