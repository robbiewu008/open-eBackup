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
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';
import { AddBackupNodeModule } from '../cluster-management/add-backup-node/add-backup-node.module';
import { BackupStorageDeviceModule } from './backup-storage-device/backup-storage-device.module';
import { BackupStorageUnitModule } from './backup-storage-unit/backup-storage-unit.module';
import { CreateDistributedNasModule } from './create-distributed-nas/create-distributed-nas.module';
import { CreateStorageDeviceComponent } from './create-storage-device/create-storage-device.component';
import { CreateStorageUnitComponent } from './create-storage-unit/create-storage-unit.component';
import { ClusterDetailModule } from './distributed-nas-detail/cluster-detail.module';
import { DistributedNasListComponent } from './distributed-nas-list.component';
import { DistributedNasListRoutingModule } from './distributed-nas-routing.module';

@NgModule({
  declarations: [
    DistributedNasListComponent,
    CreateStorageUnitComponent,
    CreateStorageDeviceComponent
  ],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ClusterDetailModule,
    ProButtonModule,
    CreateDistributedNasModule,
    SelectProtectObjectsModule,
    MultiClusterSwitchModule,
    DistributedNasListRoutingModule,
    BackupStorageUnitModule,
    AddBackupNodeModule,
    BackupStorageDeviceModule
  ],
  exports: [DistributedNasListComponent]
})
export class DistributedNasListModule {}
