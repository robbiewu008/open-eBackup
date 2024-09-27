import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { OpenstackRoutingModule } from './openstack-routing.module';
import { OpenstackComponent } from './openstack.component';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [OpenstackComponent],
  imports: [CommonModule, OpenstackRoutingModule, CopyResourceListModule]
})
export class OpenstackModule {}
