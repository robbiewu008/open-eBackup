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
import { BaseModule } from 'app/shared';
import { DesensitizationPolicyRoutingModule } from './desensitization-policy-routing.module';
import { DesensitizationPolicyComponent } from './desensitization-policy.component';
import { DesensitizationRuleModule } from './desensitization-rule/desensitization-rule.module';
import { IdentifiedRuleModule } from './identified-rule/identified-rule.module';
import { DesensitizationPolicyListModule } from './desensitization-policy-list/desensitization-policy-list.module';
import { BatchOperateServiceModule } from 'app/shared/services/batch-operate.service';
import { RelatedObjectModule } from './related-object/related-object.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [DesensitizationPolicyComponent],
  imports: [
    CommonModule,
    DesensitizationPolicyRoutingModule,
    BaseModule,
    DesensitizationRuleModule,
    IdentifiedRuleModule,
    DesensitizationPolicyListModule,
    BatchOperateServiceModule,
    RelatedObjectModule,
    MultiClusterSwitchModule
  ]
})
export class DesensitizationPolicyModule {}
