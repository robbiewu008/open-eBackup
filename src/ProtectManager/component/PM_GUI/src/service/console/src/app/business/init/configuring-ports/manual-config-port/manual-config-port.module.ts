import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule } from '@iux/live';
import { RouteConfigModule } from 'app/business/system/settings/config-network/route-config/route-config.module';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ConfigTableModule } from '../config-table/config-table.module';
import { CreatLogicPortModule } from '../creat-logic-port/creat-logic-port.module';
import { ReusePortModule } from '../reuse-port/reuse-port.module';
import { ManualConfigPortComponent } from './manual-config-port.component';

@NgModule({
  declarations: [ManualConfigPortComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    CreatLogicPortModule,
    ProButtonModule,
    RouteConfigModule,
    AlertModule,
    ConfigTableModule,
    ReusePortModule
  ],
  exports: [ManualConfigPortComponent]
})
export class ManualConfigPortModule {}
