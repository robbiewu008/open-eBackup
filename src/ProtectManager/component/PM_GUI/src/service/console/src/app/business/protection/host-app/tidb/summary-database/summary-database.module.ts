import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SummaryDatabaseComponent } from './summary-database.component';

@NgModule({
  declarations: [SummaryDatabaseComponent],
  imports: [CommonModule, BaseModule, BaseInfoModule, ProTableModule],
  exports: [SummaryDatabaseComponent]
})
export class SummaryDatabaseModule {}
