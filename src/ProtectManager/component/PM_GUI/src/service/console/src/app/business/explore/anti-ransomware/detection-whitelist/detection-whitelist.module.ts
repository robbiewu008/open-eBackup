import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetectionWhitelistComponent } from './detection-whitelist.component';
import { BaseModule } from 'app/shared';
import { BlockingRuleListModule } from '../blocking-rule-list/blocking-rule-list.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddWhitelistRuleModule } from './add-whitelist-rule/add-whitelist-rule.module';

@NgModule({
  declarations: [DetectionWhitelistComponent],
  imports: [
    CommonModule,
    BaseModule,
    BlockingRuleListModule,
    ProTableModule,
    ProButtonModule,
    AddWhitelistRuleModule
  ],
  exports: [DetectionWhitelistComponent]
})
export class DetectionWhitelistModule {}
