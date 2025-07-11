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
import { BaseModule } from 'app/shared';
import { ClickHouseComponent } from './click-house.component';
import { ClickHouseRoutingModule } from './click-house-routing.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ClusterComponent } from './cluster/cluster.component';
import { RegisterModule } from './cluster/register/register.module';
import { CreateModule } from './tabel-set/create/create.module';
import { AddNodeModule } from './cluster/add-node/add-node.module';
import { DatabaseComponent } from './database/database.component';
import { TabelSetComponent } from './tabel-set/tabel-set.component';
import { SelectTableModule } from './tabel-set/select-table/select-table.module';
import { SelectDatabaseModule } from './database/select-database/select-database.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SummaryModule as ClusterSummaryModule } from './cluster/summary/summary.module';
import { SummaryModule as DatabaseSummaryModule } from './database/summary/summary.module';
import { SummaryModule as TablesetSummaryModule } from './tabel-set/summary/summary.module';
import { DetailListModule as ClusterDetailListModule } from './cluster/detail-list/detail-list.module';
import { SelectProtectRowModule } from './tabel-set/select-protect-row/select-protect-row.module';

@NgModule({
  declarations: [
    ClickHouseComponent,
    ClusterComponent,
    DatabaseComponent,
    TabelSetComponent
  ],
  imports: [
    BaseModule,
    CommonModule,
    RegisterModule,
    CreateModule,
    AddNodeModule,
    ClusterSummaryModule,
    ClusterDetailListModule,
    SelectDatabaseModule,
    SelectProtectRowModule,
    DatabaseSummaryModule,
    TablesetSummaryModule,
    ClickHouseRoutingModule,
    MultiClusterSwitchModule,
    SelectTableModule,
    ProButtonModule,
    ProTableModule
  ]
})
export class ClickHouseModule {}
