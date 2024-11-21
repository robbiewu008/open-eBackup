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
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import {
  ProFilterSearchModule,
  ProTableModule
} from 'app/shared/components/pro-table';
import { StorageDeviceModule } from '../../../../explore/airgap/storage-device/storage-device.module';
import { BackupNodeDetailDistributedComponent } from './backup-node-detail-distributed.component';

@NgModule({
  declarations: [BackupNodeDetailDistributedComponent],
  imports: [
    CommonModule,
    BaseModule,
    AlertModule,
    ProFilterSearchModule,
    ProTableModule,
    StorageDeviceModule
  ]
})
export class BackupNodeDetailDistributedModule {}
