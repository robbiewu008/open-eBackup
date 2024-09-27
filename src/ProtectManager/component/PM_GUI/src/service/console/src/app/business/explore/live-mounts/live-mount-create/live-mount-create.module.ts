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
import { CommonModule } from '@angular/common';
import { LiveMountCreateComponent } from './live-mount-create.component';
import { BaseModule } from 'app/shared';
import { SelectCopyDataModule } from './select-copy-data/select-copy-data.module';
import { SelectResourceModule } from './select-resource/select-resource.module';
import { LiveMountOptionsModule as OracleLiveMountOptionsModule } from '../oracle/live-mount-options/live-mount-options.module';
import { LiveMountSummaryModule as OracleLiveMountSummaryModule } from '../oracle/live-mount-summary/live-mount-summary.module';
import { LiveMountOptionsModule as VMwareLiveMountOptionsModule } from '../vmware/live-mount-options/live-mount-options.module';
import { LiveMountSummaryModule as VMwareLiveMountSummaryModule } from '../vmware/live-mount-summary/live-mount-summary.module';
import { LiveMountOptionsModule as NasSharedLiveMountOptionsModule } from '../nas-shared/live-mount-options/live-mount-options.module';
import { LiveMountSummaryModule as NasSharedLiveMountSummaryModule } from '../nas-shared/live-mount-summary/live-mount-summary.module';
import { LiveMountOptionsModule as FilesetLiveMountOptionsModule } from '../fileset/live-mount-options/live-mount-options.module';
import { LiveMountSummaryModule as FilesetLiveMountSummaryModule } from '../fileset/live-mount-summary/live-mount-summary.module';
import { LiveMountOptionsModule as CnwareLiveMountOptionsModule } from '../cnware/live-mount-options/live-mount-options.module';
import { LiveMountSummaryModule as CnwareLiveMountSummaryModule } from '../cnware/live-mount-summary/live-mount-summary.module';

@NgModule({
  declarations: [LiveMountCreateComponent],
  imports: [
    CommonModule,
    BaseModule,
    SelectCopyDataModule,
    SelectResourceModule,
    OracleLiveMountOptionsModule,
    OracleLiveMountSummaryModule,
    VMwareLiveMountOptionsModule,
    VMwareLiveMountSummaryModule,
    NasSharedLiveMountOptionsModule,
    NasSharedLiveMountSummaryModule,
    FilesetLiveMountOptionsModule,
    FilesetLiveMountSummaryModule,
    CnwareLiveMountOptionsModule,
    CnwareLiveMountSummaryModule
  ],
  exports: [LiveMountCreateComponent]
})
export class LiveMountCreateModule {}
