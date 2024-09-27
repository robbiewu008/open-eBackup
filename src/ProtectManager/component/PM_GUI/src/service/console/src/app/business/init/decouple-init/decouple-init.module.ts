import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { AlertModule, TabsModule } from '@iux/live';
import { InitConfigProcessModule } from '../init-config-process/init-config-process.module';
import { ProFilterSearchModule } from 'app/shared/components/pro-table';

import { DecoupleInitComponent } from './decouple-init.component';
import { ConfigNetworkTableModule } from '../config-network-table/config-network-table.module';

@NgModule({
  declarations: [DecoupleInitComponent],
  imports: [
    CommonModule,
    BaseModule,
    AlertModule,
    InitConfigProcessModule,
    ProFilterSearchModule,
    TabsModule,
    ConfigNetworkTableModule
  ],
  exports: [DecoupleInitComponent]
})
export class DecoupleInitModule {}
