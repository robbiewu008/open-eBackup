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
import { BaseModule } from 'app/shared/base.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ManualMountModule } from 'app/shared/services/manual-mount.service';
import { CustomModalOperateModule } from '../custom-modal-operate';
import { DownloadFlrFilesModule } from '../download-flr-files/download-flr-files.module';
import { FileIndexedModule } from '../file-indexed';
import { CopyDataDetailComponent } from './copy-data-detail.component';
import { FileTreeModule } from '../file-tree/file-tree.module';

@NgModule({
  declarations: [CopyDataDetailComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    CustomModalOperateModule,
    FileIndexedModule,
    ManualMountModule,
    DownloadFlrFilesModule,
    FileTreeModule
  ],

  exports: [CopyDataDetailComponent]
})
export class CopyDataDetailModule {}
