import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../database-template/database-template.module';
import { VolumeRoutingModule } from './volume-routing.module';
import { VolumeComponent } from './volume.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { CopyDataModule } from '../database-template/copy-data/copy-data.module';
import { CreateVolumeModule } from './create-volume/create-volume.module';
import { VolumeAdvancedParameterModule } from './volume-advanced-parameter/volume-advanced-parameter.module';

@NgModule({
  declarations: [VolumeComponent],
  imports: [
    CommonModule,
    VolumeRoutingModule,
    BaseModule,
    DatabaseTemplateModule,
    MultiClusterSwitchModule,
    CreateVolumeModule,
    CopyDataModule,
    VolumeAdvancedParameterModule
  ]
})
export class VolumeModule {}
