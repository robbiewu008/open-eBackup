import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { LiveMountsListModule } from '../live-mounts-list/live-mounts-list.module';
import { VolumeRoutingModule } from './volume-routing.module';
import { VolumeComponent } from './volume.component';

@NgModule({
  declarations: [VolumeComponent],
  imports: [CommonModule, BaseModule, LiveMountsListModule, VolumeRoutingModule]
})
export class VolumeModule {}
