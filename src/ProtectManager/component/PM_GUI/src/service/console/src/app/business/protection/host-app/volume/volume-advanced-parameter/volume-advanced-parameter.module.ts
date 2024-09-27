import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { VolumeAdvancedParameterComponent } from './volume-advanced-parameter.component';
import { BaseModule } from 'app/shared';
import { ProtectFilterModule } from 'app/shared/components/protect-filter/protect-filter.module';
import { UpdateIndexModule } from 'app/shared/components/update-index/update-index.module';

@NgModule({
  declarations: [VolumeAdvancedParameterComponent],
  imports: [CommonModule, BaseModule, ProtectFilterModule, UpdateIndexModule],
  exports: [VolumeAdvancedParameterComponent]
})
export class VolumeAdvancedParameterModule {}
