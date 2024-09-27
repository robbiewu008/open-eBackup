import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { OpengaussRoutingModule } from './opengauss-routing.module';
import { OpengaussComponent } from './opengauss.component';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [OpengaussComponent],
  imports: [
    CommonModule,
    OpengaussRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class OpengaussModule {}
