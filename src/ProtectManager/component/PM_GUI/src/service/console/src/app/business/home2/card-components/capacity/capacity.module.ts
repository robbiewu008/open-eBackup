import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CapacityComponent } from './capacity.component';
import { BaseModule } from 'app/shared';
import { CapacityChartModule } from 'app/shared/components/charts/capacity-chart/capacity-chart.module';

@NgModule({
  declarations: [CapacityComponent],
  imports: [CommonModule, CapacityChartModule, BaseModule],
  exports: [CapacityComponent]
})
export class CapacityModule {}
