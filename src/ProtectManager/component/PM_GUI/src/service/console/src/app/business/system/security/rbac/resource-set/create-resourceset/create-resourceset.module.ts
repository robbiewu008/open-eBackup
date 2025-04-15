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
import { AirgapTacticsModule } from 'app/business/explore/policy/airgap/airgap-tactics.module';
import { AntiPolicyModule } from 'app/business/explore/policy/anti-policy-setting/anti-policy/anti-policy.module';
import { MountUpdatePolicyModule } from 'app/business/explore/policy/mount-update-policy/mount-update-policy.module';
import { ReportsListModule } from 'app/business/insight/reports/reports-list/reports-list.module';
import { ReportsSubscriptionModule } from 'app/business/insight/reports/reports-subscription/reports-subscription.module';
import { BaseModule } from 'app/shared';
import { ClientTemplateModule } from '../client-template/client-template.module';
import { NormalResourcesetTemplateModule } from '../normal-resourceset-template/normal-resourceset-template.module';
import { QosTemplateModule } from '../qos-template/qos-template.module';
import { SlaTemplateModule } from '../sla-template/sla-template.module';
import { VirtualCloudTemplateModule } from '../virtual-cloud-template/virtual-cloud-template.module';
import { CreateResourcesetComponent } from './create-resourceset.component';

@NgModule({
  declarations: [CreateResourcesetComponent],
  imports: [
    CommonModule,
    BaseModule,
    NormalResourcesetTemplateModule,
    VirtualCloudTemplateModule,
    ClientTemplateModule,
    SlaTemplateModule,
    QosTemplateModule,
    AirgapTacticsModule,
    AntiPolicyModule,
    ReportsListModule,
    ReportsSubscriptionModule,
    MountUpdatePolicyModule
  ],
  exports: [CreateResourcesetComponent]
})
export class CreateResourcesetModule {}
