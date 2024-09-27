import { NgModule } from '@angular/core';
import { BackupCapacityChartComponent } from './backup-capacity-chart.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  imports: [BaseModule],
  declarations: [BackupCapacityChartComponent],
  exports: [BackupCapacityChartComponent]
})
export class BackupCapacityChartModule {}
