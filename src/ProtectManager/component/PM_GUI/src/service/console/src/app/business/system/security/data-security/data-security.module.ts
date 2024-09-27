import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { DataSecurityRoutingModule } from './data-security-routing.module';
import { DataSecurityComponent } from './data-security.component';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [DataSecurityComponent],
  imports: [
    CommonModule,
    DataSecurityRoutingModule,
    BaseModule,
    MultiClusterSwitchModule
  ]
})
export class DataSecurityModule {}
