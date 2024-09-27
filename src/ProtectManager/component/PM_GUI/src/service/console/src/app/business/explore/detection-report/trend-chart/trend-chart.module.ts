import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { TrendChartComponent } from './trend-chart.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [TrendChartComponent],
  imports: [CommonModule, BaseModule],
  exports: [TrendChartComponent]
})
export class TrendChartModule {}
