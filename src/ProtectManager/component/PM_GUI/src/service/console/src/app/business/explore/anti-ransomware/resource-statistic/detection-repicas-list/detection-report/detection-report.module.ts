import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetectionReportComponent } from './detection-report.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [DetectionReportComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [DetectionReportComponent]
})
export class DetectionReportModule {}
