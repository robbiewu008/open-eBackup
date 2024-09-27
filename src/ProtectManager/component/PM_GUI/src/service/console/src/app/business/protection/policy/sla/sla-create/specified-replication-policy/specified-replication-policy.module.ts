import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ReplicationPolicyModule } from './replication-policy/replication-policy.module';
import { SpecifiedReplicationPolicyComponent } from './specified-replication-policy.component';

@NgModule({
  declarations: [SpecifiedReplicationPolicyComponent],
  imports: [CommonModule, BaseModule, ReplicationPolicyModule],
  exports: [SpecifiedReplicationPolicyComponent]
})
export class SpecifiedReplicationPolicyModule {}
