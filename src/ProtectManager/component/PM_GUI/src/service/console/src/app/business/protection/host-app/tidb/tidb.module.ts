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
import { TidbComponent } from './tidb.component';
import { TidbRoutingModule } from './tidb-routing.module';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { RegisterDatabaseModule } from './register-database/register-database.module';
import { RegisterTableModule } from './register-table/register-table.module';
import { SummaryClusterModule } from './summary-cluster/summary-cluster.module';
import { SummaryDatabaseModule } from './summary-database/summary-database.module';
import { SummaryTableModule } from './summary-table/summary-table.module';
import { TidbRestoreModule } from './tidb-restore/tidb-restore.module';
import { AdvancedParameterModule } from './advanced-parameter/advanced-parameter.module';

@NgModule({
  declarations: [TidbComponent],
  imports: [
    CommonModule,
    BaseModule,
    DatabaseTemplateModule,
    CopyDataModule,
    TidbRoutingModule,
    RegisterClusterModule,
    RegisterDatabaseModule,
    RegisterTableModule,
    SummaryClusterModule,
    SummaryDatabaseModule,
    SummaryTableModule,
    TidbRestoreModule,
    AdvancedParameterModule
  ]
})
export class TidbModule {}
