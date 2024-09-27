import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SnapshotReportComponent } from './snapshot-report.component';
import { BaseModule } from 'app/shared';
import { TrendChartModule } from '../../detection-report/trend-chart/trend-chart.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { PoReportDetailModule } from './po-report-detail/po-report-detail.module';

@NgModule({
  declarations: [SnapshotReportComponent],
  imports: [
    CommonModule,
    BaseModule,
    TrendChartModule,
    ProTableModule,
    PoReportDetailModule
  ],
  exports: [SnapshotReportComponent]
})
export class SnapshotReportModule {}
