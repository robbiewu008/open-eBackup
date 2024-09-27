import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { JobStatisticsComponent } from './job-statistics.component';
import { JobTableModule } from '../job-table/job-table.module';

@NgModule({
  declarations: [JobStatisticsComponent],
  imports: [CommonModule, BaseModule, JobTableModule],
  exports: [JobStatisticsComponent]
})
export class JobStatisticsModule {}
