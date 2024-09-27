import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { InitModule } from 'app/business/init/init.module';
import { BaseModule } from 'app/shared';
import { ConfigNetworkRoutingModule } from './config-network-routing.module';
import { ConfigNetworkComponent } from './config-network.component';
import { RouteConfigModule } from './route-config/route-config.module';

@NgModule({
  declarations: [ConfigNetworkComponent],
  imports: [
    CommonModule,
    BaseModule,
    ConfigNetworkRoutingModule,
    InitModule,
    RouteConfigModule
  ],
  exports: [ConfigNetworkComponent]
})
export class ConfigNetworkModule {}
