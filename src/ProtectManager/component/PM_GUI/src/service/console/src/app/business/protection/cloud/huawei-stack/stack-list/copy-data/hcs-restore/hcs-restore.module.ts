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
import { BaseModule } from 'app/shared/base.module';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { HCSRestoreComponent } from './hcs-restore.component';
import { BackupSetRestoreModule } from 'app/business/protection/big-data/hbase/backup-set/copy-data/backup-set-restore/backup-set-restore.module';
import { LocalFileSystemRestoreModule } from 'app/business/protection/storage/local-file-system/local-file-system-restore/local-file-system-restore.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { TargetLocationModule } from './target-location/target-location.module';

@NgModule({
  declarations: [HCSRestoreComponent],
  imports: [
    BaseModule,
    CommonModule,
    ProTableModule,
    LocalFileSystemRestoreModule,
    BackupSetRestoreModule,
    TargetLocationModule
  ]
})
export class HCSRestoreModule {}
