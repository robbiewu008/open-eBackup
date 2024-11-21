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
import { TdsqlRoutingModule } from './tdsql-routing.module';
import { TdsqlComponent } from './tdsql.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { CopyDataModule } from '../database-template/copy-data/copy-data.module';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { RegisterInstanceModule } from './register-instance/register-instance.module';
import { SummaryModule } from './summary-instance/summary.module';
import { SummaryClusterModule } from './summary-cluster/summary-cluster.module';
import { SummaryInstanceListModule } from './summary-instance-list/summary-instance-list.module';
import { SummaryModule as DistributedSummaryModule } from './dirstibuted-instance/summary/summary.module';
import { RegisterDistributedInstanceModule } from 'app/business/protection/host-app/tdsql/dirstibuted-instance/register-distributed-instance/register-distributed-instance.module';
import { RestoreModule as DistributedRestore } from './dirstibuted-instance/restore/restore.module';
@NgModule({
  declarations: [TdsqlComponent],
  imports: [
    CommonModule,
    TdsqlRoutingModule,
    BaseModule,
    DatabaseTemplateModule,
    MultiClusterSwitchModule,
    RegisterClusterModule,
    RegisterInstanceModule,
    SummaryModule,
    SummaryClusterModule,
    SummaryInstanceListModule,
    CopyDataModule,
    RegisterDistributedInstanceModule,
    DistributedSummaryModule,
    DistributedRestore
  ]
})
export class TdsqlModule {}
