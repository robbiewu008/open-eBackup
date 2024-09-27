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
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { BatchSelectSpecificDriveModule } from './batch-select-specific-drive/batch-select-specific-drive.module';
import { BatchSelectSpecificSlotModule } from './batch-specific-slot/batch-select-specific-slot.module';
import { SelectSpecificDriveModule } from './select-specific-drive/select-specific-drive.module';
import { SelectSpecificSlotModule } from './select-specific-slot/select-specific-slot.module';
import { SetBlockSizeModule } from './set-block-size/set-block-size.module';
import { StorageDeviceDetailComponent } from './storage-device-detail.component';
@NgModule({
  declarations: [StorageDeviceDetailComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    SelectSpecificDriveModule,
    SelectSpecificSlotModule,
    SetBlockSizeModule,
    BatchSelectSpecificDriveModule,
    BatchSelectSpecificSlotModule
  ],
  exports: [StorageDeviceDetailComponent]
})
export class StorageDeviceDetailModule {}
