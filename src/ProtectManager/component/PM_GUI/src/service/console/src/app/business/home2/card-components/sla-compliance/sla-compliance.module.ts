import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SlaComplianceComponent } from './sla-compliance.component';
import { CapacityChartModule } from 'app/shared/components/charts/capacity-chart/capacity-chart.module';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [SlaComplianceComponent],
  imports: [CommonModule, CapacityChartModule, BaseModule],
  exports: [SlaComplianceComponent]
})
export class SlaComplianceModule {}
