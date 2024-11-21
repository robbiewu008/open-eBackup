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
import { SummaryModule as OracleSummaryModule } from 'app/business/protection/host-app/oracle/database-list/summary/summary.module';
import { CopyDataModule as OracleCopyDataModule } from 'app/business/protection/host-app/oracle/database-list/copy-data/copy-data.module';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { CopyDataModule } from 'app/shared/components/copy-data/copy-data.module';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { ChooseDensensitizationPolicyModule } from '../choose-densensitization-policy/choose-densensitization-policy.module';
import { StartDensensitizationModule } from '../start-densensitization/start-densensitization.module';
import { DataDesensitizationListComponent } from './data-desensitization-list.component';
import { DesensitizationPolicyDetailModule } from '../../policy/desensitization-policy/desensitization-policy-list/desensitization-policy-detail/desensitization-policy-detail.module';
import { AddIdentifiedRuleModule } from '../../policy/desensitization-policy/identified-rule/add-identified-rule/add-identified-rule.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [DataDesensitizationListComponent],
  imports: [
    CommonModule,
    BaseModule,
    JobResourceModule,
    CopyDataModule,
    OracleSummaryModule,
    OracleCopyDataModule,
    BaseInfoModule,
    DetailModalModule,
    ChooseDensensitizationPolicyModule,
    StartDensensitizationModule,
    DesensitizationPolicyDetailModule,
    AddIdentifiedRuleModule,
    MultiClusterSwitchModule,
    CustomTableSearchModule
  ],
  exports: [DataDesensitizationListComponent],
  providers: [ResourceDetailService]
})
export class DataDesensitizationListModule {}
