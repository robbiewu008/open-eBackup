import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { FusionComputeBasicInfoComponent } from './fusion-compute-info.component';

@NgModule({
  declarations: [FusionComputeBasicInfoComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule
  ],
  exports: [FusionComputeBasicInfoComponent]
})
export class FusionComputeBasicInfoModule {}
