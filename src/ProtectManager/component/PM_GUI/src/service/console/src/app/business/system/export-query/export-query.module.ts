import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { ExportQueryRoutingModule } from './export-query-routing.module';
import { ExportQueryComponent } from './export-query.component';
import { ExportQueryResultsModule } from 'app/shared/components/export-query-results/export-query-results.module';

@NgModule({
  declarations: [ExportQueryComponent],
  imports: [CommonModule, ExportQueryRoutingModule, ExportQueryResultsModule]
})
export class ExportQueryModule {}
