import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { HuaWeiStackListModule } from './stack-list/huawei-stack-list.module';
import { HuaweiStackRoutingModule } from './huawei-stack-routing.module';
import { HuaweiStackComponent } from './huawei-stack.component';
import { RegisterHuaWeiStackModule } from './register-huawei-stack/register-huawei-stack.module';
import { EnvironmentInfoModule } from './stack-list/environment-info/environment-info.module';
import { CloudServerModule } from './cloud-server/cloud-server.module';
import { VirtualizationGroupModule } from '../../virtualization/virtualization-group/virtualization-group.module';

@NgModule({
  declarations: [HuaweiStackComponent],
  imports: [
    CommonModule,
    HuaweiStackRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    HuaWeiStackListModule,
    RegisterHuaWeiStackModule,
    EnvironmentInfoModule,
    CloudServerModule,
    VirtualizationGroupModule
  ]
})
export class HuaweiStackModule {}
