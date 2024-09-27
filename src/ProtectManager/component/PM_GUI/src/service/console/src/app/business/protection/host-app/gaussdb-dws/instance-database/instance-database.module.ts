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
import { RegisterModule as RegisterGbaseClusterModule } from 'app/business/protection/database/gbase/register/register.module';
import { BaseModule } from 'app/shared';
import { ModifyRetentionPolicyModule } from 'app/shared/components';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { CreateTablesetModule } from '../../db-two/create-tabelset/create-tableset.module';
import { RegisterClusterModule as RegisterDbTwoClusterModule } from '../../db-two/register-cluster/register-cluster.module';
import { RegisterInstanceModule as RegisterDbTwoInstanceModule } from '../../db-two/register-instance/register-instance.module';
import { RegisterModule as RegisterGaussdbForOpengaussModule } from '../../gaussdb-for-opengauss/register/register.module';
import { RegisterModule as RegisterGoldendbClusterModule } from '../../goldendb/register-cluster/register.module';
import { RegisterModule as RegisterGoldendbModule } from '../../goldendb/register/register.module';
import { RegisterInstanceModule as RegisterGbaseInformixInstanceModule } from '../../informix/register-instance/register-instance.module';
import { RegisterClusterModule as RegisterSQLServerClusterModule } from '../../sql-server/register-cluster/register-cluster.module';
import { CreateSchemaModule } from './create-schema/create-schema.module';
import { InstanceDatabaseComponent } from './instance-database.component';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { SelectInstanceDatabaseModule } from './select-instance-database/select-instance-database.module';
import { SummaryModule } from './summary/summary.module';
@NgModule({
  declarations: [InstanceDatabaseComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    ProtectModule,
    JobResourceModule,
    DetailModalModule,
    SlaServiceModule,
    RegisterClusterModule,
    RegisterSQLServerClusterModule,
    RegisterDbTwoClusterModule,
    RegisterDbTwoInstanceModule,
    CreateTablesetModule,
    CreateSchemaModule,
    RegisterGaussdbForOpengaussModule,
    RegisterGoldendbModule,
    RegisterGoldendbClusterModule,
    SummaryModule,
    ModifyRetentionPolicyModule,
    SelectInstanceDatabaseModule,
    MultiClusterSwitchModule,
    RegisterGbaseClusterModule,
    RegisterGbaseInformixInstanceModule
  ],
  exports: [InstanceDatabaseComponent]
})
export class InstanceDatabaseModule {}
