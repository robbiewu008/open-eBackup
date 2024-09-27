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
