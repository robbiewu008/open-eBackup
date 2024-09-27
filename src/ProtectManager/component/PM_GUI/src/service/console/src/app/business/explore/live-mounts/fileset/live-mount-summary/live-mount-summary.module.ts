import { NgModule } from '@angular/core';
import { LiveMountSummaryComponent } from './live-mount-summary.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [LiveMountSummaryComponent],
  imports: [BaseModule],
  exports: [LiveMountSummaryComponent]
})
export class LiveMountSummaryModule {}
