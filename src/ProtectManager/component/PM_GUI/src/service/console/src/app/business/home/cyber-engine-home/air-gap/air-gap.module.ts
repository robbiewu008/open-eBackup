import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AirGapComponent } from './air-gap.component';
import { BaseModule } from 'app/shared';
import { ProStatusModule } from 'app/shared/components/pro-status/pro-status.module';

@NgModule({
  imports: [CommonModule, BaseModule, ProStatusModule],
  declarations: [AirGapComponent],
  exports: [AirGapComponent]
})
export class AirGapModule {}
