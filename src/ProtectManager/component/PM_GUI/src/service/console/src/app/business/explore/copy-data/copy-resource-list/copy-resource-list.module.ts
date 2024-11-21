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
import { CopyDataModule as HostCopyDataModule } from 'app/business/protection/host-app/host/copy-data/copy-data.module';
import { SummaryModule as HostSummaryModule } from 'app/business/protection/host-app/host/summary/summary.module';
import { CopyDataModule as OracleCopyDataModule } from 'app/business/protection/host-app/oracle/database-list/copy-data/copy-data.module';
import { SummaryModule as OracleSummaryModule } from 'app/business/protection/host-app/oracle/database-list/summary/summary.module';
import { CopyDataModule as VmwareCopyDataModule } from 'app/business/protection/virtualization/vmware/vm/copy-data/copy-data.module';
import { SummaryModule as VmwareSummaryModule } from 'app/business/protection/virtualization/vmware/vm/summary/summary.module';
import { BaseModule } from 'app/shared';
import { CopyDataDetailModule } from 'app/shared/components/copy-data-detail/copy-data-detail.module';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { CopyListModule } from './copy-list/copy-list.module';
import { CopyResourceListComponent } from './copy-resource-list.component';
import { ResourceListModule } from './resource-list/resource-list.module';
import { ResourceReplicaListModule } from './resource-replica-list/resource-replica-list.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
@NgModule({
  declarations: [CopyResourceListComponent],
  imports: [
    CommonModule,
    BaseModule,
    CopyListModule,
    ResourceListModule,
    DetailModalModule,
    HostSummaryModule,
    JobResourceModule,
    HostCopyDataModule,
    OracleCopyDataModule,
    OracleSummaryModule,
    CopyDataDetailModule,
    VmwareCopyDataModule,
    VmwareSummaryModule,
    ResourceReplicaListModule,
    MultiClusterSwitchModule
  ],
  exports: [CopyResourceListComponent],
  providers: [ResourceDetailService]
})
export class CopyResourceListModule {}
