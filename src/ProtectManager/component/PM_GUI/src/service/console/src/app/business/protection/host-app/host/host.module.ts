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
import { CopyDataModule } from 'app/business/protection/host-app/host/copy-data/copy-data.module';
import { SummaryModule } from 'app/business/protection/host-app/host/summary/summary.module';
import { BaseModule } from 'app/shared';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { TakeManualBackupModule } from 'app/shared/components/take-manual-backup/take-manual-backup.module';
import { WarningBatchConfirmModule } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.module';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { TakeManualBackupServiceModule } from 'app/shared/services/take-manual-backup.service';
import { ConfigLogLevelModule } from './config-log-level/config-log-level.module';
import { DownloadProxyModule } from './download-proxy/download-proxy.module';
import { HostRoutingModule } from './host-routing.module';
import { HostComponent } from './host.component';
import { LanFreeModule } from './lan-free/lan-free.module';
import { ModifyHostModule } from './modify-host/modify-host.module';
import { UpdateAgentModule } from './update-agent/update-agent.module';
import { AddTagModule } from './add-tag/add-tag.module';
import { ModifyResourceModule } from './modify-resource/modify-resource.module';
import { ModifyAzModule } from './modify-az/modify-az.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';
import { LinkModule } from '@iux/live';

@NgModule({
  declarations: [HostComponent],
  imports: [
    CommonModule,
    HostRoutingModule,
    BaseModule,
    DetailModalModule,
    SummaryModule,
    JobResourceModule,
    CopyDataModule,
    ProtectModule,
    TakeManualBackupModule,
    TakeManualBackupServiceModule,
    DownloadProxyModule,
    ModifyHostModule,
    UpdateAgentModule,
    WarningBatchConfirmModule,
    MultiClusterSwitchModule,
    ConfigLogLevelModule,
    LanFreeModule,
    AddTagModule,
    ModifyResourceModule,
    ModifyAzModule,
    CustomTableSearchModule,
    LinkModule
  ],
  providers: [ResourceDetailService]
})
export class HostModule {}
