import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ReportDetailRouterModule } from './report-detail.routing';
import { ReportDetailComponent } from './report-detail.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { BaseModule } from 'app/shared';
import { AlertModule } from '@iux/live';
import { CapacityTrendChartModule } from '../explore/detection-report/capacity-trend-chart/capacity-trend-chart.module';
import { TrendChartModule } from '../explore/detection-report/trend-chart/trend-chart.module';

@NgModule({
  imports: [
    CommonModule,
    ReportDetailRouterModule,
    BaseModule,
    ProTableModule,
    ProStatusModule,
    AlertModule,
    TrendChartModule,
    CapacityTrendChartModule
  ],
  declarations: [ReportDetailComponent]
})
export class ReportDetailModule {}
