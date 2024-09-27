import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { HyperVRoutingModule } from './hyper-v-routing.module';
import { HyperVComponent } from './hyper-v.component';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [HyperVComponent],
  imports: [CommonModule, HyperVRoutingModule, CopyResourceListModule],
  exports: [HyperVComponent]
})
export class HyperVModule {}
