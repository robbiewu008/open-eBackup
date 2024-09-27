import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { FusionComputeBasicInfoModule } from '../../fusion-compute/fusion-list/environment-info/basic-info/fusion-compute-basic-info.module';
import { EnvironmentInfoComponent } from './environment-info.component';

@NgModule({
  declarations: [EnvironmentInfoComponent],
  imports: [
    CommonModule,
    BaseModule,
    FusionComputeBasicInfoModule,
    CustomModalOperateModule
  ]
})
export class EnvironmentInfoModule {}
