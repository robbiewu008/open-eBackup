import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { JobDetailComponent } from './job-detail.component';
import { JobEventModule } from './job-event/job-event.module';
import { JobStrategyModule } from './job-strategy/job-strategy.module';
import { ReportResultModule } from './report-result/report-result.module';

@NgModule({
  declarations: [JobDetailComponent],
  imports: [
    CommonModule,
    BaseModule,
    JobEventModule,
    ReportResultModule,
    JobStrategyModule
  ],
  exports: [JobDetailComponent]
})
export class JobDetailModule {}
