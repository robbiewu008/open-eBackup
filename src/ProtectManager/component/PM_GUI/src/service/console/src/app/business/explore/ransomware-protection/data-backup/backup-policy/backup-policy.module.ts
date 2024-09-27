import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { BackupPolicyComponent } from './backup-policy.component';
import { CreateBackupPolicyModule } from './create-backup-policy/create-backup-policy.module';

@NgModule({
  declarations: [BackupPolicyComponent],
  imports: [CommonModule, BaseModule, ProTableModule, CreateBackupPolicyModule],
  exports: [BackupPolicyComponent]
})
export class BackupPolicyModule {}
