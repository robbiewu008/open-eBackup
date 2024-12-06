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
import { BackupSetRestoreModule } from 'app/business/protection/big-data/hbase/backup-set/copy-data/backup-set-restore/backup-set-restore.module';
import { LocalFileSystemRestoreModule } from 'app/business/protection/storage/local-file-system/local-file-system-restore/local-file-system-restore.module';
import { BaseModule } from 'app/shared/base.module';
import { ProTableModule } from '../pro-table';
import { FileLevelRestoreComponent } from './file-level-restore.component';
import { ManualInputPathModule } from '../manual-input-path/manual-input-path.module';

@NgModule({
  declarations: [FileLevelRestoreComponent],
  imports: [
    BaseModule,
    CommonModule,
    LocalFileSystemRestoreModule,
    BackupSetRestoreModule,
    ProTableModule,
    ManualInputPathModule
  ],
  exports: [FileLevelRestoreComponent]
})
export class FileLevelRestoreModule {}
