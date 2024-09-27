import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import {
  IdentifiedRuleComponent,
  SelectionPipe
} from './identified-rule.component';
import { BaseModule } from 'app/shared';
import { AddIdentifiedRuleModule } from './add-identified-rule/add-identified-rule.module';
import { RelatedDesensitizationPolicyModule } from './related-desensitization-policy/related-desensitization-policy.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [IdentifiedRuleComponent, SelectionPipe],
  imports: [
    CommonModule,
    BaseModule,
    AddIdentifiedRuleModule,
    RelatedDesensitizationPolicyModule,
    CustomTableSearchModule
  ],
  exports: [IdentifiedRuleComponent]
})
export class IdentifiedRuleModule {}
