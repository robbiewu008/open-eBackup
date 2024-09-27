import { NgModule } from '@angular/core';
import { JobChartComponent } from './job-chart.component';
import {
  DropdownModule,
  IconModule,
  ButtonModule,
  GroupModule
} from '@iux/live';
import { BaseModule } from 'app/shared/base.module';
import { RouterModule } from '@angular/router';

@NgModule({
  imports: [
    BaseModule,
    DropdownModule,
    IconModule,
    ButtonModule,
    GroupModule,
    RouterModule
  ],
  declarations: [JobChartComponent],
  exports: [JobChartComponent]
})
export class JobChartModule {}
