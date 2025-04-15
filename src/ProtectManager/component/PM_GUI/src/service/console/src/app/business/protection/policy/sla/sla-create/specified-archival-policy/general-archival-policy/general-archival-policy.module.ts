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
import { CreateArchiveStorageModule } from 'app/business/system/infrastructure/archive-storage/create-archive-storage/create-archive-storage.module';
import { CreateStoragePoolModule } from 'app/business/system/infrastructure/archive-storage/storage-pool-list/create-storage-pool/create-storage-pool.module';
import { BaseModule } from 'app/shared';
import { CurrentSystemTimeModule } from 'app/shared/components/current-system-time/current-system-time.module';
import { GeneralArchivalPolicyComponent } from './general-archival-policy.component';
import { RouterJumpHelpModule } from 'app/shared/components/router-jump-help/router-jump-help.module';

@NgModule({
  declarations: [GeneralArchivalPolicyComponent],
  imports: [
    CommonModule,
    BaseModule,
    CreateArchiveStorageModule,
    CreateStoragePoolModule,
    CurrentSystemTimeModule,
    RouterJumpHelpModule
  ],
  exports: [GeneralArchivalPolicyComponent]
})
export class GeneralArchivalPolicyModule {}
