import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { PerformanceRoutingModule } from './performance-routing.module';
import { PerformanceComponent } from './performance.component';
import { BaseModule } from 'app/shared';
import { AlertModule, MenuModule } from '@iux/live';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { MultiSettingModule } from './multi-setting/multi-setting.module';

@NgModule({
  declarations: [PerformanceComponent],
  imports: [
    CommonModule,
    PerformanceRoutingModule,
    BaseModule,
    MenuModule,
    MultiClusterSwitchModule,
    MultiSettingModule,
    AlertModule
  ]
})
export class PerformanceModule {}
