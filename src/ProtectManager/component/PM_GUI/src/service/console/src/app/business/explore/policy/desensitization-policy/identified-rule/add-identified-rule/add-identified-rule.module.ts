import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddIdentifiedRuleComponent } from './add-identified-rule.component';
import { BaseModule } from 'app/shared';
import { AddRuleModule } from '../../desensitization-rule/add-rule/add-rule.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [AddIdentifiedRuleComponent],
  imports: [CommonModule, BaseModule, AddRuleModule, CustomTableSearchModule],
  exports: [AddIdentifiedRuleComponent]
})
export class AddIdentifiedRuleModule {}
