import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { CapacityDictionComponent } from './capacity-diction.component';
import { CapacityDictionChartComponent } from '../../../../shared/components/charts/capacity-diction-chart/capacity-diction-chart.component';

@NgModule({
  declarations: [CapacityDictionComponent, CapacityDictionChartComponent],
  imports: [CommonModule, BaseModule],
  exports: [CapacityDictionComponent]
})
export class CapacityDictionModule {}
