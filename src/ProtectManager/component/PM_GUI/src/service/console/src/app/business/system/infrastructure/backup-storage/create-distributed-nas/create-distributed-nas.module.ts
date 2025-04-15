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
import {
  AlertModule,
  DatatableModule,
  PaginatorModule,
  TransferModule
} from '@iux/live';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';
import { SetStoragePolicyModule } from '../set-storage-policy/set-storage-policy.module';
import { CreateDistributedNasComponent } from './create-distributed-nas.component';
@NgModule({
  declarations: [CreateDistributedNasComponent],
  imports: [
    CommonModule,
    BaseModule,
    SelectProtectObjectsModule,
    ProTableModule,
    DatatableModule,
    PaginatorModule,
    TransferModule,
    SetStoragePolicyModule,
    AlertModule
  ]
})
export class CreateDistributedNasModule {}
