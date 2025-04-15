/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { JobResourceModule } from 'app/business/insight/job/job-resource/job-resource.module';
import { BaseModule } from 'app/shared';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { EnvironmentInfoModule } from './environment-info/environment-info.module';
import { RegisterVmModule } from './register-vm/register-vm.module';
import { VmListModule } from './vm-list/vm-list.module';
import { VmwareRoutingModule } from './vmware-routing.module';
import { VmwareComponent } from './vmware.component';
import { VirtualizationGroupModule } from '../virtualization-group/virtualization-group.module';

@NgModule({
  declarations: [VmwareComponent],
  imports: [
    CommonModule,
    VmwareRoutingModule,
    BaseModule,
    ProtectModule,
    VmListModule,
    JobResourceModule,
    RegisterVmModule,
    DetailModalModule,
    MultiClusterSwitchModule,
    EnvironmentInfoModule,
    VirtualizationGroupModule
  ],
  exports: [VmwareComponent],
  providers: [ResourceDetailService]
})
export class VmwareModule {}
