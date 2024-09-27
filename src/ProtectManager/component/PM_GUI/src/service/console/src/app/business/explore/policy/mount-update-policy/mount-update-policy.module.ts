import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { CreateUpdatePolicyModule } from './create-update-policy/create-update-policy.module';
import { MountUpdatePolicyRoutingModule } from './mount-update-policy-routing.module';
import { MountUpdatePolicyComponent } from './mount-update-policy.component';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [MountUpdatePolicyComponent],
  imports: [
    CommonModule,
    MountUpdatePolicyRoutingModule,
    BaseModule,
    CreateUpdatePolicyModule,
    MultiClusterSwitchModule,
    CustomTableSearchModule
  ],
  exports: [MountUpdatePolicyComponent]
})
export class MountUpdatePolicyModule {}
