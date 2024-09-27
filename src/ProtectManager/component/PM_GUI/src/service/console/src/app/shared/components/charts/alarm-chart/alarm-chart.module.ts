import { NgModule } from '@angular/core';
import { AlarmChartComponent } from './alarm-chart.component';
import { BaseModule } from 'app/shared/base.module';
import { RouterModule } from '@angular/router';

@NgModule({
  imports: [BaseModule, RouterModule],
  declarations: [AlarmChartComponent],
  exports: [AlarmChartComponent]
})
export class AlarmChartModule {}
