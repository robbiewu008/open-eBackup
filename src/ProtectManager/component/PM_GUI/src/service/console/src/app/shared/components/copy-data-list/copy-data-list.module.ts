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
import { ModalService } from '@iux/live';
import { BaseModule } from 'app/shared/base.module';
import { CopyDataDetailModule } from '../copy-data-detail/copy-data-detail.module';
import { FileIndexedModule } from '../file-indexed/file-indexed.module';
import { ModifyRetentionPolicyModule } from '../modify-retention-policy/modify-retention-policy.module';
import { CopyDataListComponent } from './copy-data-list.component';
import { RestoreModule } from 'app/shared/services/restore.service';
import { ManualMountModule } from 'app/shared/services/manual-mount.service';
import { ManualIndexModule } from '../manual-index/manual-index.module';
import { CopyActionModule } from 'app/shared/services/copy-action.service';
import { CopyVerifyModule } from '../copy-verify-proxy/copy-verify.module';
import { CustomTableSearchModule } from '../custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [CopyDataListComponent],
  imports: [
    CommonModule,
    BaseModule,
    FileIndexedModule,
    CopyDataDetailModule,
    ModifyRetentionPolicyModule,
    RestoreModule,
    ManualMountModule,
    ManualIndexModule,
    CopyActionModule,
    CopyVerifyModule,
    CustomTableSearchModule
  ],
  exports: [CopyDataListComponent],
  providers: [ModalService]
})
export class CopyDataListModule {}
