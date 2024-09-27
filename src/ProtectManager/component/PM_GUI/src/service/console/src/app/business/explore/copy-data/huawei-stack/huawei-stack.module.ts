import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { HuaweiStackRoutingModule } from './huawei-stack-routing.module';
import { HuaweiStackComponent } from './huawei-stack.component';

@NgModule({
  declarations: [HuaweiStackComponent],
  imports: [
    CommonModule,
    HuaweiStackRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class HuaweiStackModule {}
