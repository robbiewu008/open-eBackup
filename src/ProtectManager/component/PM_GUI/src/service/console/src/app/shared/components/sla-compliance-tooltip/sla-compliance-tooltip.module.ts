import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SlaComplianceTooltipComponent } from './sla-compliance-tooltip.component';
import { IconModule, TooltipModule } from '@iux/live';

@NgModule({
  declarations: [SlaComplianceTooltipComponent],
  imports: [CommonModule, TooltipModule, IconModule],
  exports: [SlaComplianceTooltipComponent]
})
export class SlaComplianceTooltipModule {}
