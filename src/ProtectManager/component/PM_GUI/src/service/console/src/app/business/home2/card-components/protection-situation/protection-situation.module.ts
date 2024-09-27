import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { ProtectionSituationComponent } from './protection-situation.component';
import { ProtectionChartModule } from 'app/shared/components/charts/protection-chart/protection-chart.module';

@NgModule({
  declarations: [ProtectionSituationComponent],
  imports: [BaseModule, CommonModule, ProtectionChartModule],
  exports: [ProtectionSituationComponent]
})
export class ProtectionSituationModule {}
