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
import { InstanceDatabaseModule } from '../gaussdb-dws/instance-database/instance-database.module';
import { SQLServerRoutingModule } from './sql-server-routing.module';
import { SQLServerComponent } from './sql-server.component';
import { RegisterInstanceModule } from './register-instance/register-instance.module';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { SummaryModule } from './summary/summary.module';
import { SelectInstanceDatabaseModule } from './select-instance-database/select-instance-database.module';
import { ClusterDetailModule } from './cluster-detail/cluster-detail.module';
import { ClusterBackupsetDetailModule } from './cluster-backupset-detail/cluster-backupset-detail.module';
@NgModule({
  declarations: [SQLServerComponent],
  imports: [
    CommonModule,
    SQLServerRoutingModule,
    BaseModule,
    InstanceDatabaseModule,
    RegisterInstanceModule,
    MultiClusterSwitchModule,
    SelectInstanceDatabaseModule,
    ClusterDetailModule,
    ClusterBackupsetDetailModule,
    SummaryModule
  ]
})
export class SQLServerModule {}
