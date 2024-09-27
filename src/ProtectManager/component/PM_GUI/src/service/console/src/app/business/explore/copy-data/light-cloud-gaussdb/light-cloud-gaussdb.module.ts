import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { LightCloudGaussdbRoutingModule } from './light-cloud-gaussdb-routing.module';
import { LightCloudGaussdbComponent } from './light-cloud-gaussdb.component';

@NgModule({
  declarations: [LightCloudGaussdbComponent],
  imports: [
    CommonModule,
    LightCloudGaussdbRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class LightCloudGaussdbModule {}
