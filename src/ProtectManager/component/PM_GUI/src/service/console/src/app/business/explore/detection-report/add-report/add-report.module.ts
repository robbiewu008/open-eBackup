import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CommonModule } from '@angular/common';
import { AddReportComponent } from './add-report.component';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  imports: [CommonModule, BaseModule, ProTableModule],
  declarations: [AddReportComponent],
  exports: [AddReportComponent]
})
export class AddReportModule {}
