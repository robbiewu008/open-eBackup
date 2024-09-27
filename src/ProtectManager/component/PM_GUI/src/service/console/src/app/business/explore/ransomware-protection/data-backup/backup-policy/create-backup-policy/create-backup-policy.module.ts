import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateBackupPolicyComponent } from './create-backup-policy.component';
import { BaseModule } from 'app/shared';
import { DetectUpperBoundModule } from '../detect-upper-bound/detect-upper-bound.module';

@NgModule({
  declarations: [CreateBackupPolicyComponent],
  imports: [CommonModule, BaseModule, DetectUpperBoundModule],
  exports: [CreateBackupPolicyComponent]
})
export class CreateBackupPolicyModule {}
