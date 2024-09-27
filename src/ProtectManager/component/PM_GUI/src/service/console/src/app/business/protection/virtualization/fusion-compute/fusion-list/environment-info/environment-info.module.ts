import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { FusionComputeBasicInfoModule } from './basic-info/fusion-compute-basic-info.module';
import { EnvironmentInfoComponent } from './environment-info.component';

@NgModule({
  declarations: [EnvironmentInfoComponent],
  imports: [BaseModule, FusionComputeBasicInfoModule, CustomModalOperateModule]
})
export class EnvironmentInfoModule {}
