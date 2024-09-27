import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule, TransferModule } from '@iux/live';
import { RouteConfigModule } from 'app/business/system/settings/config-network/route-config/route-config.module';
import { BaseModule } from 'app/shared';
import { AddNetworkComponent } from './add-network.component';

@NgModule({
  declarations: [AddNetworkComponent],
  imports: [
    CommonModule,
    BaseModule,
    TransferModule,
    AlertModule,
    RouteConfigModule
  ],
  exports: [AddNetworkComponent]
})
export class AddNetworkModule {}
