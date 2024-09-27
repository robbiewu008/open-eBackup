import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';
import { ConfigTableModule } from './config-table/config-table.module';
import { ConfiguringPortsComponent } from './configuring-ports.component';
import { CreatLogicPortModule } from './creat-logic-port/creat-logic-port.module';
import { RouteTableModule } from './route-table/route-table.module';

@NgModule({
  declarations: [ConfiguringPortsComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    CreatLogicPortModule,
    ProButtonModule,
    RouteTableModule,
    ConfigTableModule
  ],
  exports: [ConfiguringPortsComponent]
})
export class ConfiguringPortsModule {}
