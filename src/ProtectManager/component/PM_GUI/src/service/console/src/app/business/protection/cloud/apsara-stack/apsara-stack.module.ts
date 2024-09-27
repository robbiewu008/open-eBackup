import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { VirtualizationBaseModule } from '../../virtualization/virtualization-base/virtualization-base.module';
import { ApsaraStackRoutingModule } from './apsara-stack-routing.module';
import { ApsaraStackComponent } from './apsara-stack.component';
import { ResourceSetSummaryModule } from './resource-set-summary/resource-set-summary.module';

@NgModule({
  declarations: [ApsaraStackComponent],
  imports: [
    CommonModule,
    ApsaraStackRoutingModule,
    VirtualizationBaseModule,
    BaseModule,
    ResourceSetSummaryModule
  ],
  exports: [ApsaraStackComponent]
})
export class ApsaraStackModule {}
