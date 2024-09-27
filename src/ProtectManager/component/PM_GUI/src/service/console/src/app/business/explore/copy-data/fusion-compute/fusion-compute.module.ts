import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { FusionComputeRoutingModule } from './fusion-compute-routing.module';
import { FusionComputeComponent } from './fusion-compute.component';

@NgModule({
  declarations: [FusionComputeComponent],
  imports: [
    CommonModule,
    FusionComputeRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class FusionComputeModule {}
