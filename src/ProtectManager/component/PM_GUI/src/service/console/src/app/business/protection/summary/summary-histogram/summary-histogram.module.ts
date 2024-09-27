import { NgModule } from '@angular/core';
import { SummaryHistogramComponent } from './summary-histogram.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [SummaryHistogramComponent],
  imports: [BaseModule],
  exports: [SummaryHistogramComponent]
})
export class SummaryHistogramModule {}
