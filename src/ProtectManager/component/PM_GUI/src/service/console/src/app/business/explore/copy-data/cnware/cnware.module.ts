import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CnwareRoutingModule } from './cnware-routing.module';
import { CnwareComponent } from './cnware.component';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [CnwareComponent],
  imports: [
    CommonModule,
    CnwareRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class CnwareModule {}
