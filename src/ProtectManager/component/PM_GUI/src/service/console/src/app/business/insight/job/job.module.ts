import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { JobFilterModule } from './job-filter/job-filter.module';
import { JobRoutingModule } from './job-routing.module';
import { JobStatisticsModule } from './job-statistics/job-statistics.module';
import { JobTableModule } from './job-table/job-table.module';
import { JobComponent } from './job.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { JobResourceModule } from './job-resource/job-resource.module';

@NgModule({
  declarations: [JobComponent],
  imports: [
    CommonModule,
    JobRoutingModule,
    BaseModule,
    JobTableModule,
    JobFilterModule,
    JobStatisticsModule,
    MultiClusterSwitchModule,
    JobResourceModule
  ]
})
export class JobModule {}
