import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SummaryComponent } from './summary.component';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { TablesModule } from '../tables/tables.module';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [SummaryComponent],
  imports: [
    CommonModule,
    BaseModule,
    BaseInfoModule,
    TablesModule,
    ProTableModule
  ],
  exports: [SummaryComponent]
})
export class SummaryModule {}
