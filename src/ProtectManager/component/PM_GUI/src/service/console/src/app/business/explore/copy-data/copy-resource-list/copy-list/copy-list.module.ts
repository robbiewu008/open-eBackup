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
import { FileIndexedModule } from 'app/shared/components';
import { RestoreModule } from 'app/shared/services/restore.service';
import { CopyListComponent, FilterPipe } from './copy-list.component';
import { ManualMountModule } from 'app/shared/services/manual-mount.service';
import { ManualIndexModule } from 'app/shared/components/manual-index/manual-index.module';
import { CopyActionModule } from 'app/shared/services/copy-action.service';
import { CopyVerifyModule } from 'app/shared/components/copy-verify-proxy/copy-verify.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';
import { CustomTableFilterModule } from '../../../../../shared/components/custom-table-filter/custom-table-filter.module';

@NgModule({
  declarations: [CopyListComponent, FilterPipe],
  imports: [
    CommonModule,
    BaseModule,
    FileIndexedModule,
    RestoreModule,
    ManualMountModule,
    ManualIndexModule,
    CopyActionModule,
    CopyVerifyModule,
    CustomTableSearchModule,
    CustomTableFilterModule
  ],
  exports: [CopyListComponent]
})
export class CopyListModule {}
