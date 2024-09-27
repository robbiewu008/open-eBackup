import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from '../multi-cluster-switch/multi-cluster-switch.module';
import { ProTableModule } from '../pro-table';
import { ExportQueryResultsComponent } from './export-query-results.component';

@NgModule({
  declarations: [ExportQueryResultsComponent],
  imports: [CommonModule, BaseModule, MultiClusterSwitchModule, ProTableModule],

  exports: [ExportQueryResultsComponent]
})
export class ExportQueryResultsModule {}
