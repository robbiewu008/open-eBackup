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
import { BaseModule } from 'app/shared/base.module';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { CopyDataModule } from 'app/shared/components/copy-data/copy-data.module';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { TakeManualBackupComponent } from 'app/shared/components/take-manual-backup/take-manual-backup.component';
import { TakeManualBackupModule } from 'app/shared/components/take-manual-backup/take-manual-backup.module';
import { DatabaseListModule } from './database-list/database-list.module';
import { OracleRoutingModule } from './oracle-routing.module';
import { OracleComponent } from './oracle.component';
import { HostClusterListModule } from './host-cluster-list/host-cluster-list.module';
import { SummaryModule } from './host-cluster-list/summary/summary.module';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [OracleComponent],
  imports: [
    CommonModule,
    OracleRoutingModule,
    BaseModule,
    DatabaseListModule,
    HostClusterListModule,
    JobResourceModule,
    CopyDataModule,
    BaseInfoModule,
    DetailModalModule,
    TakeManualBackupModule,
    SummaryModule,
    MultiClusterSwitchModule
  ],
  providers: [ResourceDetailService]
})
export class OracleModule {}
