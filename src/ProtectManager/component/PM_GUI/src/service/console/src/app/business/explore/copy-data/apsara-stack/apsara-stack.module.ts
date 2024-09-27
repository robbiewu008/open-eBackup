import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { ApsaraStackRoutingModule } from './apsara-stack-routing.module';
import { ApsaraStackComponent } from './apsara-stack.component';

@NgModule({
  declarations: [ApsaraStackComponent],
  imports: [
    CommonModule,
    BaseModule,
    ApsaraStackRoutingModule,
    CopyResourceListModule
  ]
})
export class ApsaraStackModule {}
