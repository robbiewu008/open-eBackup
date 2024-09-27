import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ProtectionChartComponent } from './protection-chart.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [ProtectionChartComponent],
  imports: [CommonModule, BaseModule],
  exports: [ProtectionChartComponent]
})
export class ProtectionChartModule {}
