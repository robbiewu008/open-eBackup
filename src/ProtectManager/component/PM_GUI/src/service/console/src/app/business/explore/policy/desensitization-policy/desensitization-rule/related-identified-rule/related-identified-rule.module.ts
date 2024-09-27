import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RelatedIdentifiedRuleComponent } from './related-identified-rule.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [RelatedIdentifiedRuleComponent],
  imports: [CommonModule, BaseModule],
  exports: [RelatedIdentifiedRuleComponent]
})
export class RelatedIdentifiedRuleModule {}
