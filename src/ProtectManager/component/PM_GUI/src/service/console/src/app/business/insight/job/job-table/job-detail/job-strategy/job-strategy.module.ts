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
import { LiveMountSummaryModule as cnwareLiveMountSummaryModule } from 'app/business/explore/live-mounts/cnware/live-mount-summary/live-mount-summary.module';
import { LiveMountSummaryModule as filesetLiveMountSummaryModule } from 'app/business/explore/live-mounts/fileset/live-mount-summary/live-mount-summary.module';
import { LiveMountSummaryModule as NasLiveMountSummaryModule } from 'app/business/explore/live-mounts/nas-shared/live-mount-summary/live-mount-summary.module';
import { LiveMountSummaryModule as oracleLiveMountSummaryModule } from 'app/business/explore/live-mounts/oracle/live-mount-summary/live-mount-summary.module';
import { LiveMountSummaryModule as vmwareLiveMountSummaryModule } from 'app/business/explore/live-mounts/vmware/live-mount-summary/live-mount-summary.module';
import { BaseModule } from 'app/shared';
import { SlaInfoModule } from 'app/shared/components';
import { RestoreParameterDetailModule } from '../restore-parameter-detail/restore-parameter-detail.module';
import { JobStrategyComponent } from './job-strategy.component';

@NgModule({
  declarations: [JobStrategyComponent],
  imports: [
    CommonModule,
    BaseModule,
    SlaInfoModule,
    filesetLiveMountSummaryModule,
    oracleLiveMountSummaryModule,
    NasLiveMountSummaryModule,
    vmwareLiveMountSummaryModule,
    cnwareLiveMountSummaryModule,
    RestoreParameterDetailModule
  ],
  exports: [JobStrategyComponent]
})
export class JobStrategyModule {}
