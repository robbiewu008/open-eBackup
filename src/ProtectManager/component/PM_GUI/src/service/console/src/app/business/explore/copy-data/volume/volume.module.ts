import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { VolumeRoutingModule } from './volume-routing.module';
import { VolumeComponent } from './volume.component';

@NgModule({
  declarations: [VolumeComponent],
  imports: [
    CommonModule,
    VolumeRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class VolumeModule {}
