import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { SummaryHistogramModule } from './summary-histogram/summary-histogram.module';
import { SummaryRoutingModule } from './summary-routing.module';
import { SummaryComponent } from './summary.component';

@NgModule({
  declarations: [SummaryComponent],
  imports: [
    BaseModule,
    SummaryRoutingModule,
    SummaryHistogramModule,
    MultiClusterSwitchModule
  ]
})
export class SummaryModule {}
