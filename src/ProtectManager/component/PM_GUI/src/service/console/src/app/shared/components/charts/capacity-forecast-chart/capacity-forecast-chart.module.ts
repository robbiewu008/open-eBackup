import { NgModule } from '@angular/core';
import { CapacityForecastChartComponent } from './capacity-forecast-chart.component';
import { BaseModule } from 'app/shared/base.module';
import { RouterModule } from '@angular/router';

@NgModule({
  imports: [BaseModule, RouterModule],
  declarations: [CapacityForecastChartComponent],
  exports: [CapacityForecastChartComponent]
})
export class CapacityForecastChartModule {}
