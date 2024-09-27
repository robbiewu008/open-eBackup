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
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { DatabaseTemplateModule } from '../database-template/database-template.module';
import { LightCloudGaussdbRoutingModule } from './light-cloud-gaussdb-routing.module';
import { LightCloudGaussdbComponent } from './light-cloud-gaussdb.component';
import { SummaryModule } from './summary/summary.module';
import { SummaryModule as ProjectSummaryModule } from './project-summary/summary.module';
import { RegisterModule } from './register/register.module';

@NgModule({
  declarations: [LightCloudGaussdbComponent],
  imports: [
    CommonModule,
    LightCloudGaussdbRoutingModule,
    BaseModule,
    DatabaseTemplateModule,
    SummaryModule,
    ProjectSummaryModule,
    MultiClusterSwitchModule,
    RegisterModule
  ]
})
export class LightCloudGaussdbModule {}
