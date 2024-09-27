import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AntiPolicyDetailModule } from './anti-policy-detail/anti-policy-detail.module';
import { AntiPolicyComponent } from './anti-policy.component';
import { CreateAntiPolicyModule } from './create-anti-policy/create-anti-policy.module';

@NgModule({
  declarations: [AntiPolicyComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    CreateAntiPolicyModule,
    AntiPolicyDetailModule,
    MultiClusterSwitchModule
  ],
  exports: [AntiPolicyComponent]
})
export class AntiPolicyModule {}
