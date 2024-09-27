import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetectionRepicasListComponent } from './detection-repicas-list.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { DetectionReportModule } from './detection-report/detection-report.module';
import { FeedbackWarningModule } from './feedback-warning/feedback-warning.module';

@NgModule({
  declarations: [DetectionRepicasListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    DetectionReportModule,
    FeedbackWarningModule
  ],
  exports: [DetectionRepicasListComponent]
})
export class DetectionRepicasListModule {}
