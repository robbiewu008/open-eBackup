import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SystemTimeRoutingModule } from './system-time-routing.module';
import { SystemTimeComponent } from './system-time.component';
import { CurrentSystemTimeModule } from 'app/shared/components/current-system-time/current-system-time.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [SystemTimeComponent],
  exports: [SystemTimeComponent],
  imports: [
    CommonModule,
    SystemTimeRoutingModule,
    BaseModule,
    CurrentSystemTimeModule,
    MultiClusterSwitchModule
  ]
})
export class SystemTimeModule {}
