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
import { BatchOperateServiceModule } from 'app/shared/services/batch-operate.service';
import { ArchiveStorageDetailModule } from './archive-storage-detail/archive-storage-detail.module';
import { StorageManagementRoutingModule } from './archive-storage-routing.module';
import {
  ArchiveStorageComponent,
  SelectionPipe
} from './archive-storage.component';
import { CreateArchiveStorageModule } from './create-archive-storage/create-archive-storage.module';
import { ModifyAlarmThresholdModule } from './modify-alarm-threshold/modify-alarm-threshold.module';
import { StoragePoolListModule } from './storage-pool-list/storage-pool-list.module';
import { StorageDeviceListModule } from './storage-device-list/storage-device-list.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [ArchiveStorageComponent, SelectionPipe],
  imports: [
    CommonModule,
    BaseModule,
    StoragePoolListModule,
    StorageDeviceListModule,
    StorageManagementRoutingModule,
    CreateArchiveStorageModule,
    ModifyAlarmThresholdModule,
    ArchiveStorageDetailModule,
    BatchOperateServiceModule,
    MultiClusterSwitchModule
  ]
})
export class ArchiveStorageModule {}
