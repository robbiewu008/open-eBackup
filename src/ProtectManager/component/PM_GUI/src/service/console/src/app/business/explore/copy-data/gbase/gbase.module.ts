import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { GbaseRoutingModule } from './gbase-routing.module';
import { GbaseComponent } from './gbase.component';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [GbaseComponent],
  imports: [
    CommonModule,
    GbaseRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class GbaseModule {}
