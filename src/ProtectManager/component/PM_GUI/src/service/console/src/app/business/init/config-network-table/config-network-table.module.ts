import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ConfigNetworkTableComponent } from './config-network-table.component';
import { BaseModule } from 'app/shared';
import { AlertModule } from '@iux/live';
import { ProFilterSearchModule } from 'app/shared/components/pro-table';
import { InitConfigProcessModule } from '../init-config-process/init-config-process.module';

@NgModule({
  declarations: [ConfigNetworkTableComponent],
  imports: [
    CommonModule,
    BaseModule,
    AlertModule,
    InitConfigProcessModule,
    ProFilterSearchModule
  ],
  exports: [ConfigNetworkTableComponent]
})
export class ConfigNetworkTableModule {}
