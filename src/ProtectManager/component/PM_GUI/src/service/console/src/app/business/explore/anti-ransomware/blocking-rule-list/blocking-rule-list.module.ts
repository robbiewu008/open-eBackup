import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BlockingRuleListComponent } from './blocking-rule-list.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddBlockingRuleModule } from './add-blocking-rule/add-blocking-rule.module';
import { AssociateVstoreModule } from './associate-vstore/associate-vstore.module';
import { BlocingRuleListRoutingModule } from './blocing-rule-list-routing.module';

@NgModule({
  declarations: [BlockingRuleListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    AddBlockingRuleModule,
    AssociateVstoreModule,
    BlocingRuleListRoutingModule
  ]
})
export class BlockingRuleListModule {}
