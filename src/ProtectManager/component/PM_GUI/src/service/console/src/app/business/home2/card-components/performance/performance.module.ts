import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { PerformanceComponent } from './performance.component';
import { PerformanceTimeChartComponent } from '../../../../shared/components/charts/performance-time-chart/performance-time-chart.component';

@NgModule({
  declarations: [PerformanceComponent, PerformanceTimeChartComponent],
  imports: [CommonModule, BaseModule],
  exports: [PerformanceComponent]
})
export class PerformanceModule {}
