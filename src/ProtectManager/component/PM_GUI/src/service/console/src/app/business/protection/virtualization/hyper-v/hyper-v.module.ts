import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';

import { VirtualizationBaseModule } from '../virtualization-base/virtualization-base.module';
import { HyperVRoutingModule } from './hyper-v-routing.module';
import { HyperVComponent } from './hyper-v.component';
import { HypervCopyDataModule } from './hyperv-copy-data/hyperv-copy-data.module';
import { SummaryModule } from './summary/summary.module';

@NgModule({
  declarations: [HyperVComponent],
  imports: [
    CommonModule,
    HyperVRoutingModule,
    VirtualizationBaseModule,
    SummaryModule,
    HypervCopyDataModule
  ],
  exports: [HyperVComponent]
})
export class HyperVModule {}
