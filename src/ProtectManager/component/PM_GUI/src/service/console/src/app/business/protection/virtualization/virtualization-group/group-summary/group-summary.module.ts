import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { GroupSummaryComponent } from './group-summary.component';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [GroupSummaryComponent],
  imports: [
    CommonModule,
    BaseModule,
    BaseInfoModule,
    ProTableModule,
    ProButtonModule
  ]
})
export class GroupSummaryModule {}
