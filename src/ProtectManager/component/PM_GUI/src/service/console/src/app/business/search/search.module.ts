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
import { NgModule } from '@angular/core';
import { JobResourceModule } from 'app/business/insight/job/job-resource/job-resource.module';
import { BaseModule } from 'app/shared';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { TakeManualBackupServiceModule } from 'app/shared/services/take-manual-backup.service';
import { CopyDataModule as HostCopyDataModule } from '../protection/host-app/host/copy-data/copy-data.module';
import { SummaryModule as HostSummaryModule } from '../protection/host-app/host/summary/summary.module';
import { AuthModule as OracleAuthModule } from '../protection/host-app/oracle/database-list/auth/auth.module';
import { CopyDataModule as OracleCopyDataModule } from '../protection/host-app/oracle/database-list/copy-data/copy-data.module';
import { SummaryModule as OracleSummaryModule } from '../protection/host-app/oracle/database-list/summary/summary.module';
import { CopyDataModule as VmwareCopyDataModule } from 'app/business/protection/virtualization/vmware/vm/copy-data/copy-data.module';
import { SummaryModule as VMSummaryModule } from 'app/business/protection/virtualization/vmware/vm/summary/summary.module';
import { SummaryModule as VmwareSummaryModule } from 'app/business/protection/virtualization/vmware/summary/summary.module';
import { SummaryModule as HyperVSummaryModule } from 'app/business/protection/virtualization/hyper-v/summary/summary.module';
import { FileListModule } from './file-list/file-list.module';
import { ResourceListModule } from './resource-list/resource-list.module';
import { SearchRoutingModule } from './search-routing.module';
import { SearchComponent } from './search.component';
import { CopyDataDetailModule } from 'app/shared/components/copy-data-detail/copy-data-detail.module';
import { CnwareModule } from '../protection/virtualization/cnware/cnware.module';

const HostModules = [HostSummaryModule, HostCopyDataModule];
const VMwareModules = [
  VmwareCopyDataModule,
  VmwareSummaryModule,
  VMSummaryModule
];
const OracleModules = [
  OracleSummaryModule,
  OracleCopyDataModule,
  OracleAuthModule
];

@NgModule({
  declarations: [SearchComponent],
  imports: [
    BaseModule,
    SearchRoutingModule,
    ResourceListModule,
    FileListModule,
    BaseModule,
    JobResourceModule,
    DetailModalModule,
    ProtectModule,
    CopyDataDetailModule,
    TakeManualBackupServiceModule,
    ...HostModules,
    ...OracleModules,
    ...VMwareModules,
    CnwareModule,
    HyperVSummaryModule
  ],
  providers: [ResourceDetailService]
})
export class GlobalSearchModule {}
