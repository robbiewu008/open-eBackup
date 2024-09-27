import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { LimitRatePolicyRoutingModule } from './limit-rate-policy-routing.module';
import { LimitRatePolicyComponent } from './limit-rate-policy.component';
import { BaseModule } from 'app/shared';
import { CreateLimitRatePolicyModule } from './create-limit-rate-policy/create-limit-rate-policy.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [LimitRatePolicyComponent],
  imports: [
    CommonModule,
    LimitRatePolicyRoutingModule,
    BaseModule,
    CreateLimitRatePolicyModule,
    MultiClusterSwitchModule,
    CustomTableSearchModule
  ]
})
export class LimitRatePolicyModule {}
