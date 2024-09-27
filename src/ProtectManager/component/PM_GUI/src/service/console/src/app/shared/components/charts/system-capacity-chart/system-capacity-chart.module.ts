import { NgModule } from '@angular/core';
import { SystemCapacityChartComponent } from './system-capacity-chart.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  imports: [BaseModule],
  declarations: [SystemCapacityChartComponent],
  exports: [SystemCapacityChartComponent]
})
export class SystemCapacityChartModule {}
