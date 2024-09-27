import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LiveMountSummaryComponent } from './live-mount-summary.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [LiveMountSummaryComponent],
  imports: [CommonModule, BaseModule],
  exports: [LiveMountSummaryComponent]
})
export class LiveMountSummaryModule {}
