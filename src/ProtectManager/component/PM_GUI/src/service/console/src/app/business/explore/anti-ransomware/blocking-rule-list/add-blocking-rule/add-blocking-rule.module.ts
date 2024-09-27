import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddBlockingRuleComponent } from './add-blocking-rule.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [AddBlockingRuleComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [AddBlockingRuleComponent]
})
export class AddBlockingRuleModule {}
