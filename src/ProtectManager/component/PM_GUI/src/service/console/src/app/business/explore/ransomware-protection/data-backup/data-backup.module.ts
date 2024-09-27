import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { DataBackupRoutingModule } from './data-backup-routing.module';
import { DataBackupComponent } from './data-backup.component';
import { BaseModule } from 'app/shared';
import { FileSystemModule } from './file-system/file-system.module';
import { BackupPolicyModule } from './backup-policy/backup-policy.module';

@NgModule({
  declarations: [DataBackupComponent],
  imports: [
    CommonModule,
    DataBackupRoutingModule,
    BaseModule,
    FileSystemModule,
    BackupPolicyModule
  ]
})
export class DataBackupModule {}
