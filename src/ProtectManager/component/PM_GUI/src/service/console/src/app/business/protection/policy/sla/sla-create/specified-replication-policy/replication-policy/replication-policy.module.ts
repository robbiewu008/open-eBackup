import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AddTargetClusterModule } from 'app/business/system/infrastructure/cluster-management/add-target-cluster/add-target-cluster.module';
import { AddStorageModule } from 'app/business/system/infrastructure/external-storage/add-storage/add-storage.module';
import { BaseModule } from 'app/shared';
import { CurrentSystemTimeModule } from 'app/shared/components/current-system-time/current-system-time.module';
import { ReplicationPolicyComponent } from './replication-policy.component';

@NgModule({
  declarations: [ReplicationPolicyComponent],
  imports: [
    CommonModule,
    BaseModule,
    AddStorageModule,
    CurrentSystemTimeModule,
    AddTargetClusterModule
  ],
  exports: [ReplicationPolicyComponent]
})
export class ReplicationPolicyModule {}
