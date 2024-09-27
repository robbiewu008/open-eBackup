import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { FusionOneRoutingModule } from './fusion-one-routing.module';
import { FusionOneComponent } from './fusion-one.component';
import { FusionComputeModule } from '../fusion-compute/fusion-compute.module';

@NgModule({
  declarations: [FusionOneComponent],
  imports: [CommonModule, FusionOneRoutingModule, FusionComputeModule]
})
export class FusionOneModule {}
