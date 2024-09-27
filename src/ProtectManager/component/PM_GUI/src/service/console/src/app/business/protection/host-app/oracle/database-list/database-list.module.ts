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
