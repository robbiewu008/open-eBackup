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
import { CopyDataModule } from '../../host-app/database-template/copy-data/copy-data.module';
import { DatabaseTemplateModule } from '../../host-app/database-template/database-template.module';
import { BaseModule } from 'app/shared';
import { SaphanaRoutingModule } from './saphana-routing.module';
import { SaphanaComponent } from './saphana.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { RegisterInstanceModule } from './register-instance/register-instance.module';
import { RegisterDatabaseModule } from './register-database/register-database.module';
import { SummaryInstanceModule } from './summary-instance/summary-instance.module';
import { SummaryDatabaseListModule } from './summary-database-list/summary-database-list.module';
import { SummaryDatabaseModule } from './summary-database/summary-database.module';

@NgModule({
  declarations: [SaphanaComponent],
  imports: [
    CommonModule,
    SaphanaRoutingModule,
    BaseModule,
    DatabaseTemplateModule,
    MultiClusterSwitchModule,
    RegisterInstanceModule,
    RegisterDatabaseModule,
    SummaryInstanceModule,
    SummaryDatabaseListModule,
    SummaryDatabaseModule,
    CopyDataModule
  ]
})
export class SaphanaModule {}
