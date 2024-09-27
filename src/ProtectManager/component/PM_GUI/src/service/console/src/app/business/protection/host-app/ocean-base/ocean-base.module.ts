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
import { DatabaseTemplateModule } from '../database-template/database-template.module';
import { CopyDataModule } from '../database-template/copy-data/copy-data.module';
import { OceanBaseRoutingModule } from './ocean-base-routing.module';
import { OceanBaseComponent } from './ocean-base.component';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { RegisterTenantModule } from './register-tenant/register-tenant.module';
import { SummaryClusterModule } from './summary-cluster/summary-cluster.module';
import { SummaryTenantListModule } from './summary-tenant-list/summary-tenant-list.module';
import { SummaryTenantModule } from './summary-tenant/summary-tenant.module';
@NgModule({
  declarations: [OceanBaseComponent],
  imports: [
    CommonModule,
    BaseModule,
    DatabaseTemplateModule,
    OceanBaseRoutingModule,
    RegisterClusterModule,
    RegisterTenantModule,
    SummaryClusterModule,
    SummaryTenantListModule,
    SummaryTenantModule,
    CopyDataModule
  ]
})
export class OceanBaseModule {}
