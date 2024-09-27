import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ButtonModule, SelectModule } from '@iux/live';
import { SlaComplianceTooltipModule } from 'app/shared/components/sla-compliance-tooltip/sla-compliance-tooltip.module';
import { CardComponent } from './card.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [CardComponent],
  imports: [
    BaseModule,
    CommonModule,
    ButtonModule,
    SlaComplianceTooltipModule,
    SelectModule
  ],
  exports: [CardComponent]
})
export class CardModule {}
