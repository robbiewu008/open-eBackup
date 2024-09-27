import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { OceanBaselRoutingModule } from './ocean-base-routing.module';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { OceanBaseComponent } from './ocean-base.component';

@NgModule({
  declarations: [OceanBaseComponent],
  imports: [
    CommonModule,
    BaseModule,
    OceanBaselRoutingModule,
    CopyResourceListModule
  ]
})
export class OceanBaseModule {}
