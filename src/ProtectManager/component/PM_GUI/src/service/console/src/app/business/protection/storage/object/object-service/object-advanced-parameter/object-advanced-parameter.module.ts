import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ObjectAdvancedParameterComponent } from './object-advanced-parameter.component';
import { UpdateIndexModule } from 'app/shared/components/update-index/update-index.module';

@NgModule({
  declarations: [ObjectAdvancedParameterComponent],
  imports: [CommonModule, BaseModule, UpdateIndexModule],
  exports: [ObjectAdvancedParameterComponent]
})
export class ObjectAdvancedParameterModule {}
