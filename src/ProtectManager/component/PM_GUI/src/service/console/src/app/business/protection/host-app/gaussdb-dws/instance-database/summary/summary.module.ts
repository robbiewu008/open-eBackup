import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SummaryComponent } from './summary.component';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { TablesModule } from 'app/business/protection/big-data/hbase/backup-set/tables/tables.module';

@NgModule({
  declarations: [SummaryComponent],
  imports: [
    CommonModule,
    BaseModule,
    BaseInfoModule,
    ProTableModule,
    TablesModule
  ],
  exports: [SummaryComponent]
})
export class SummaryModule {}
