import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DesensitizationPolicyRoutingModule } from './desensitization-policy-routing.module';
import { DesensitizationPolicyComponent } from './desensitization-policy.component';
import { DesensitizationRuleModule } from './desensitization-rule/desensitization-rule.module';
import { IdentifiedRuleModule } from './identified-rule/identified-rule.module';
import { DesensitizationPolicyListModule } from './desensitization-policy-list/desensitization-policy-list.module';
import { BatchOperateServiceModule } from 'app/shared/services/batch-operate.service';
import { RelatedObjectModule } from './related-object/related-object.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [DesensitizationPolicyComponent],
  imports: [
    CommonModule,
    DesensitizationPolicyRoutingModule,
    BaseModule,
    DesensitizationRuleModule,
    IdentifiedRuleModule,
    DesensitizationPolicyListModule,
    BatchOperateServiceModule,
    RelatedObjectModule,
    MultiClusterSwitchModule
  ]
})
export class DesensitizationPolicyModule {}
