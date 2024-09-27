import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { HostTrustworthinessComponent } from './host-trustworthiness.component';
import { HostTrustworthinessRoutingModule } from './host-trustworthiness-routing.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { BaseModule } from 'app/shared';

@NgModule({
  imports: [
    CommonModule,
    HostTrustworthinessRoutingModule,
    BaseModule,
    MultiClusterSwitchModule
  ],
  declarations: [HostTrustworthinessComponent]
})
export class HostTrustworthinessModule {}
