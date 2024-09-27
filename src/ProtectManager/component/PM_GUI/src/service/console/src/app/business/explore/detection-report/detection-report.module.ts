import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { DetectionReportRoutingModule } from './detection-report-routing.module';
import { BaseModule } from 'app/shared';
import { DetectionReportComponent } from './detection-report.component';
import { ListItemModule } from './list-item/list-item.module';
import { AddReportModule } from './add-report/add-report.module';
import { PaginatorModule } from '@iux/live';

@NgModule({
  declarations: [DetectionReportComponent],
  imports: [
    CommonModule,
    DetectionReportRoutingModule,
    BaseModule,
    ListItemModule,
    AddReportModule,
    PaginatorModule
  ]
})
export class DetectionReportModule {}
