import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import {
  DesensitizationRuleComponent,
  SelectionPipe
} from './desensitization-rule.component';
import { BaseModule } from 'app/shared';
import { AddRuleModule } from './add-rule/add-rule.module';
import { RelatedIdentifiedRuleModule } from './related-identified-rule/related-identified-rule.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [DesensitizationRuleComponent, SelectionPipe],
  imports: [
    CommonModule,
    BaseModule,
    AddRuleModule,
    RelatedIdentifiedRuleModule,
    CustomTableSearchModule
  ],
  exports: [DesensitizationRuleComponent]
})
export class DesensitizationRuleModule {}
