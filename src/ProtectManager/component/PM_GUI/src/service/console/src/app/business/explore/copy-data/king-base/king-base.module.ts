import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { KingBaseRoutingModule } from './king-base-routing.module';
import { KingBaseComponent } from './king-base.component';

@NgModule({
  declarations: [KingBaseComponent],
  imports: [
    CommonModule,
    KingBaseRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class KingBaseModule {}
