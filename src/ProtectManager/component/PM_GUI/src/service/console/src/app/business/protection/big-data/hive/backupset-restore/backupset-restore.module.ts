import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { BackupsetRestoreComponent } from './backupset-restore.component';

@NgModule({
  declarations: [BackupsetRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [BackupsetRestoreComponent]
})
export class BackupsetRestoreModule {}
