import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ReportResultComponent } from './report-result.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ReportResultComponent],
  imports: [CommonModule, BaseModule]
})
export class ReportResultModule {}
