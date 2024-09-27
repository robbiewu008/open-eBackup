import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { VmwareRoutingModule } from './vmware-routing.module';
import { VmwareComponent } from './vmware.component';

@NgModule({
  declarations: [VmwareComponent],
  imports: [
    CommonModule,
    VmwareRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class VmwareModule {}
