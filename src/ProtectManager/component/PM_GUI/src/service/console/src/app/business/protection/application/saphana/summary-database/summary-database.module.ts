import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SummaryDatabaseComponent } from './summary-database.component';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [SummaryDatabaseComponent],
  imports: [
    CommonModule,
    BaseModule,
    BaseInfoModule,
    ProTableModule,
    ProStatusModule
  ],
  exports: [SummaryDatabaseComponent]
})
export class SummaryDatabaseModule {}
