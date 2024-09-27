import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SelectApplicationModule } from './select-application/select-application.module';
import { SlaCreateComponent } from './sla-create.component';
import { SpecifiedArchivalPolicyModule } from './specified-archival-policy/specified-archival-policy.module';
import { SpecifiedBackupPolicyModule } from './specified-backup-policy/specified-backup-policy.module';
import { SpecifiedReplicationPolicyModule } from './specified-replication-policy/specified-replication-policy.module';
@NgModule({
  imports: [
    CommonModule,
    BaseModule,
    SelectApplicationModule,
    SpecifiedBackupPolicyModule,
    SpecifiedArchivalPolicyModule,
    SpecifiedReplicationPolicyModule
  ],
  declarations: [SlaCreateComponent],

  exports: [SlaCreateComponent]
})
export class SlaCreateModule {}
