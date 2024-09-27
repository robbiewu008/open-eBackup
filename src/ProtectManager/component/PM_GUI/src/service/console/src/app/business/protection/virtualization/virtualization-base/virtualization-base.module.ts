import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { EnvironmentInfoApsaraStackModule } from '../../cloud/apsara-stack/environment-info-apsara-stack/environment-info-apsara-stack.module';
import { RegisterApsaraStackModule } from '../../cloud/apsara-stack/register-apsara-stack/register-apsara-stack.module';
import { RegisterVmModule } from '../vmware/register-vm/register-vm.module';
import { BaseTableModule } from './base-table/base-table.module';
import { EnvironmentInfoModule } from './environment-info/environment-info.module';
import { VirtualizationBaseComponent } from './virtualization-base.component';
import { RegisterModule as HyperVRegisterModule } from '../hyper-v/register/register.module';
@NgModule({
  declarations: [VirtualizationBaseComponent],
  imports: [
    CommonModule,
    BaseModule,
    MultiClusterSwitchModule,
    ProButtonModule,
    BaseTableModule,
    EnvironmentInfoModule,
    RegisterVmModule,
    RegisterApsaraStackModule,
    HyperVRegisterModule,
    EnvironmentInfoApsaraStackModule
  ],
  exports: [VirtualizationBaseComponent]
})
export class VirtualizationBaseModule {}
