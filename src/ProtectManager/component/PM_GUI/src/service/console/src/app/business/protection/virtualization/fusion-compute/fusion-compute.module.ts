import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { FusionComputeComponent } from './fusion-compute.component';
import { FusionComputeRoutingModule } from './fusion-compute-routing.module';
import { LiveMountsListModule } from 'app/business/explore/live-mounts/live-mounts-list/live-mounts-list.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { FusionListModule } from './fusion-list/fusion-list.module';
import { RegisterFusionComputeModule } from './register-fusion-compute/register-fusion-compute.module';
import { EnvironmentInfoModule } from './fusion-list/environment-info/environment-info.module';
import { VirtualizationGroupModule } from '../virtualization-group/virtualization-group.module';

@NgModule({
  declarations: [FusionComputeComponent],
  imports: [
    CommonModule,
    FusionComputeRoutingModule,
    BaseModule,
    LiveMountsListModule,
    MultiClusterSwitchModule,
    RegisterFusionComputeModule,
    FusionListModule,
    EnvironmentInfoModule,
    VirtualizationGroupModule
  ],
  exports: [FusionComputeComponent]
})
export class FusionComputeModule {}
