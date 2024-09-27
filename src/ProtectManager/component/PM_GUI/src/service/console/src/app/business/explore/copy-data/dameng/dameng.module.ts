import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { DamengRoutingModule } from './dameng-routing.module';
import { DamengComponent } from './dameng.component';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [DamengComponent],
  imports: [
    CommonModule,
    DamengRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class DamengModule {}
