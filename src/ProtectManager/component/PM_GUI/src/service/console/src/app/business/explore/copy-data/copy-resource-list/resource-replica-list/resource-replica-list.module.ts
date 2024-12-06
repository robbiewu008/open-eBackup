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
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { ManualCopyModule } from './manual-copy/manual-copy.module';
import { ResourceReplicaListComponent } from './resource-replica-list.component';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';
import { ModifyOwnedUserModule } from 'app/shared/components/modify-owned-user/modify-owned-user.module';

@NgModule({
  declarations: [ResourceReplicaListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProtectModule,
    ManualCopyModule,
    CustomTableSearchModule,
    ModifyOwnedUserModule
  ],
  exports: [ResourceReplicaListComponent]
})
export class ResourceReplicaListModule {}
