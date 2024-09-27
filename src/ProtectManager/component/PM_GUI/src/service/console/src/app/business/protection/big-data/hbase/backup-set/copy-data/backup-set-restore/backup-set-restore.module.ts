import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BackupSetRestoreComponent } from './backup-set-restore.component';

@NgModule({
  declarations: [BackupSetRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [BackupSetRestoreComponent]
})
export class BackupSetRestoreModule {}
