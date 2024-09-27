import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BackupSetRestoreModule } from 'app/business/protection/big-data/hbase/backup-set/copy-data/backup-set-restore/backup-set-restore.module';
import { LocalFileSystemRestoreModule } from 'app/business/protection/storage/local-file-system/local-file-system-restore/local-file-system-restore.module';
import { BaseModule } from 'app/shared/base.module';
import { ProTableModule } from '../pro-table';
import { FileLevelRestoreComponent } from './file-level-restore.component';

@NgModule({
  declarations: [FileLevelRestoreComponent],
  imports: [
    BaseModule,
    CommonModule,
    LocalFileSystemRestoreModule,
    BackupSetRestoreModule,
    ProTableModule
  ],
  exports: [FileLevelRestoreComponent]
})
export class FileLevelRestoreModule {}
