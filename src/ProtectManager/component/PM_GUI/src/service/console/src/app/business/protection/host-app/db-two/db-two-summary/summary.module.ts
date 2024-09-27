import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SummaryComponent } from './summary.component';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { TablesModule } from 'app/business/protection/big-data/hbase/backup-set/tables/tables.module';

@NgModule({
  declarations: [SummaryComponent],
  imports: [
    CommonModule,
    BaseModule,
    BaseInfoModule,
    ProTableModule,
    ProStatusModule,
    TablesModule
  ],
  exports: [SummaryComponent]
})
export class SummaryModule {}
