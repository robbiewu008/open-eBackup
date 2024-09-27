import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';
import { SummaryTableComponent } from './summary-table.component';

@NgModule({
  declarations: [SummaryTableComponent],
  imports: [CommonModule, BaseModule, ProTableModule, BaseInfoModule],
  exports: [SummaryTableComponent]
})
export class SummaryTableModule {}
