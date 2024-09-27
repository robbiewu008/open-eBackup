import { NgModule } from '@angular/core';
import { PerformanceChartComponent } from './performance-chart.component';
import { GroupModule, SelectModule } from '@iux/live';
import { FormsModule } from '@angular/forms';
import { BaseModule } from 'app/shared/base.module';
import { RouterModule } from '@angular/router';

@NgModule({
  imports: [BaseModule, GroupModule, SelectModule, FormsModule, RouterModule],
  declarations: [PerformanceChartComponent],
  exports: [PerformanceChartComponent]
})
export class PerformanceChartModule {}
