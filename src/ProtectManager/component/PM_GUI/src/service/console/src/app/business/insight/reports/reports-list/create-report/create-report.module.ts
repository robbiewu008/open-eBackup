import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateReportComponent } from './create-report.component';
import { BaseModule } from 'app/shared';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [CreateReportComponent],
  imports: [CommonModule, BaseModule, AlertModule]
})
export class CreateReportModule {}
