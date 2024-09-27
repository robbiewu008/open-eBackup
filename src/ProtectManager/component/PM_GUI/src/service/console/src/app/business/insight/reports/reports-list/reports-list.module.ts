import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ReportsListComponent } from './reports-list.component';
import { CreateReportModule } from './create-report/create-report.module';

@NgModule({
  declarations: [ReportsListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    CreateReportModule
  ],
  exports: [ReportsListComponent]
})
export class ReportsListModule {}
