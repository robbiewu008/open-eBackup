import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { VmwareRoutingModule } from './vmware-routing.module';
import { VmwareComponent } from './vmware.component';
import { LiveMountsListModule } from '../live-mounts-list/live-mounts-list.module';

@NgModule({
  declarations: [VmwareComponent],
  imports: [CommonModule, VmwareRoutingModule, BaseModule, LiveMountsListModule]
})
export class VmwareModule {}
