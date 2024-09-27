import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { StorageSummaryComponent } from './storage-summary.component';
import { BaseModule } from 'app/shared';
import { ThresholdModifyModule } from './threshold-modify/threshold-modify.module';

@NgModule({
  declarations: [StorageSummaryComponent],
  imports: [CommonModule, BaseModule, ThresholdModifyModule],
  exports: [StorageSummaryComponent]
})
export class StorageSummaryModule {}
