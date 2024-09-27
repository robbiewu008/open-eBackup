import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SlaComplianceChartComponent } from './sla-compliance-chart.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [SlaComplianceChartComponent],
  imports: [CommonModule, BaseModule],
  exports: [SlaComplianceChartComponent]
})
export class SlaComplianceChartModule {}
