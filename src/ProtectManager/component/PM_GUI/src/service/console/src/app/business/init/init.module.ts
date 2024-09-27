import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { ConfiguringPortsModule } from './configuring-ports/configuring-ports.module';
import { ManualConfigPortModule } from './configuring-ports/manual-config-port/manual-config-port.module';
import { InitConfigProcessModule } from './init-config-process/init-config-process.module';
import { InitRoutingModule } from './init-routing.module';
import { InitComponent } from './init.component';
import { DistributedInitModule } from './distributed-init/distributed-init.module';
import { DecoupleInitModule } from './decouple-init/decouple-init.module';

@NgModule({
  declarations: [InitComponent],
  imports: [
    CommonModule,
    InitRoutingModule,
    BaseModule,
    InitConfigProcessModule,
    ConfiguringPortsModule,
    ManualConfigPortModule,
    DistributedInitModule,
    DecoupleInitModule,
    AlertModule
  ],
  exports: [InitComponent]
})
export class InitModule {}
