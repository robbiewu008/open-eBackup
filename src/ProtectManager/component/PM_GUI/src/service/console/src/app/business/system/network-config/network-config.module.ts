import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ModifyNetworkProcessModule } from './modify-network-process/modify-network-process.module';
import { NetworkConfigRoutingModule } from './network-config-routing.module';
import { NetworkConfigComponent } from './network-config.component';
import { IpConfigListModule } from './ip-config-list/ip-config-list.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { DetailModule } from './detail/detail.module';
import { BindRelationModule } from './bind-relation/bind-relation.module';

@NgModule({
  declarations: [NetworkConfigComponent],
  imports: [
    CommonModule,
    NetworkConfigRoutingModule,
    BaseModule,
    IpConfigListModule,
    ModifyNetworkProcessModule,
    MultiClusterSwitchModule,
    ProTableModule,
    ProButtonModule,
    DetailModule,
    BindRelationModule
  ]
})
export class NetworkConfigModule {}
