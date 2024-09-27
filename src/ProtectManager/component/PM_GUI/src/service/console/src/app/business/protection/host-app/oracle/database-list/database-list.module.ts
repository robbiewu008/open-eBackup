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
import { NgModule } from '@angular/core';
import { OracleSingleFileRestoreModule } from 'app/business/protection/host-app/oracle/database-list/restore/single-file-restore/oracle-single-file-restore.module';
import { OracleTableLevelRestoreModule } from './restore/table-level-restore/table-level-restore.module';
import { BaseModule } from 'app/shared/base.module';
import { DatabaseListComponent } from './database-list.component';
import { CommonModule } from '@angular/common';
import { SummaryModule } from './summary/summary.module';
import { CopyDataModule } from './copy-data/copy-data.module';
import { AuthModule } from './auth/auth.module';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { TakeManualBackupServiceModule } from 'app/shared/services/take-manual-backup.service';
import { WarningBatchConfirmModule } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.module';
import { RegisterModule } from './register/register.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [DatabaseListComponent],
  imports: [
    CommonModule,
    BaseModule,
    SummaryModule,
    CopyDataModule,
    AuthModule,
    ProtectModule,
    TakeManualBackupServiceModule,
    WarningBatchConfirmModule,
    RegisterModule,
    CustomTableSearchModule,
    OracleSingleFileRestoreModule,
    OracleTableLevelRestoreModule
  ],
  exports: [DatabaseListComponent]
})
export class DatabaseListModule {}
