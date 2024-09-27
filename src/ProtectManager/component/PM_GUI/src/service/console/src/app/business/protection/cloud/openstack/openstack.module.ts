import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { DomainListModule } from './domain-list/domain-list.module';
import { EnvironmentInfoModule } from './environment-info/environment-info.module';
import { OpenstackListModule } from './openstack-list/openstack-list.module';
import { OpenstackRoutingModule } from './openstack-routing.module';
import { OpenstackComponent } from './openstack.component';
import { RegisterOpenstackModule } from './register-openstack/register-openstack.module';
import { VirtualizationGroupModule } from '../../virtualization/virtualization-group/virtualization-group.module';

@NgModule({
  declarations: [OpenstackComponent],
  imports: [
    CommonModule,
    OpenstackRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    ProTableModule,
    ProButtonModule,
    OpenstackListModule,
    RegisterOpenstackModule,
    EnvironmentInfoModule,
    DomainListModule,
    VirtualizationGroupModule
  ]
})
export class OpenstackModule {}
