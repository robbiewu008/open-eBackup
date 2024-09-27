import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateFilterRuleComponent } from './create-filter-rule.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [CreateFilterRuleComponent],
  imports: [CommonModule, BaseModule],
  exports: [CreateFilterRuleComponent]
})
export class CreateFilterRuleModule {}
