import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared/base.module';
import { CoupleNodeChartComponent } from './couple-node-chart.component';

@NgModule({
  declarations: [CoupleNodeChartComponent],
  imports: [CommonModule, BaseModule],
  exports: [CoupleNodeChartComponent]
})
export class CoupleNodeChartModule {}
