import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { SnmpTrapRoutingModule } from './snmp-trap-routing.module';
import { AddTrapIpComponent, SnmpTrapComponent } from './snmp-trap.component';
import { SnmpV3EngineModule } from './snmp-v3-engine/snmp-v3-engine.module';

@NgModule({
  declarations: [SnmpTrapComponent, AddTrapIpComponent],
  imports: [
    SnmpTrapRoutingModule,
    BaseModule,
    SnmpV3EngineModule,
    MultiClusterSwitchModule
  ],
  providers: []
})
export class SnmpTrapModule {}
