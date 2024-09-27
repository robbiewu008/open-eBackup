import { NgModule } from '@angular/core';
import { ArchiveCapacityChartComponent } from './archive-capacity-chart.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  imports: [BaseModule],
  declarations: [ArchiveCapacityChartComponent],
  exports: [ArchiveCapacityChartComponent]
})
export class ArchiveCapacityChartModule {}
