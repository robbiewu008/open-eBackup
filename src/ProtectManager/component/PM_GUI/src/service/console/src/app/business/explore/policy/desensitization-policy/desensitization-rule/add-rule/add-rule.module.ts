import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddRuleComponent } from './add-rule.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AddRuleComponent],
  imports: [CommonModule, BaseModule],
  exports: [AddRuleComponent]
})
export class AddRuleModule {}
