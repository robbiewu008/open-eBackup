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
import { AdvancedParametersModule } from '@backup-policy/advanced-parameters/advanced-parameters.module';
import { GeneralAdvancedParameterModule } from '@backup-policy/general-advanced-parameter/general-advanced-parameter.module';
import { ProtectParameterDetailModule } from 'app/business/insight/job/job-table/job-detail/protect-parameter-detail/protect-parameter-detail.module';
import { HyperVParameterModule } from 'app/business/protection/policy/sla/sla-create/specified-backup-policy/hyper-v-parameter/hyper-v-parameter.module';
import { LocalFileSystemAdvancedParameterModule } from 'app/business/protection/policy/sla/sla-create/specified-backup-policy/local-file-system-advanced-parameter/local-file-system-advanced-parameter.module';
import { VmwareAdvancedParameterModule } from 'app/business/protection/policy/sla/sla-create/specified-backup-policy/vmware-advanced-parameter/vmware-advanced-parameter.module';
import { SlaAssociateResourceModule } from 'app/business/protection/policy/sla/sla-detail/sla-associate-resource/sla-associate-resource.module';
import { BaseModule } from './../../base.module';
import { SlaInfoComponent } from './sla-info.component';

@NgModule({
  declarations: [SlaInfoComponent],
  imports: [
    CommonModule,
    BaseModule,
    HyperVParameterModule,
    VmwareAdvancedParameterModule,
    LocalFileSystemAdvancedParameterModule,
    GeneralAdvancedParameterModule,
    AdvancedParametersModule,
    SlaAssociateResourceModule,
    ProtectParameterDetailModule
  ],
  exports: [SlaInfoComponent]
})
export class SlaInfoModule {}
