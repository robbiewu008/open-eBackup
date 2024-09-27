import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { SecuritypolicyRoutingModule } from './security-policy-routing.module';
import { SecuritypolicyComponent } from './security-policy.component';
import { SwitchModule, TooltipModule, InputModule } from '@iux/live';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [SecuritypolicyComponent],
  imports: [
    CommonModule,
    BaseModule,
    TooltipModule,
    SwitchModule,
    SecuritypolicyRoutingModule,
    InputModule,
    MultiClusterSwitchModule
  ]
})
export class SecuritypolicyModule {}
