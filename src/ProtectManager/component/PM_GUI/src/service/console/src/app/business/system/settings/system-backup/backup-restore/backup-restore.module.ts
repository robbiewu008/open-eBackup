import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BackupRestoreComponent } from './backup-restore.component';

@NgModule({
  declarations: [BackupRestoreComponent],
  imports: [CommonModule, BaseModule]
})
export class BackupRestoreModule {}
