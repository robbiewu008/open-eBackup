import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BackupPolicyComponent } from './backup-policy.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [BackupPolicyComponent],
  imports: [CommonModule, BaseModule],
  exports: [BackupPolicyComponent]
})
export class BackupPolicyModule {}
