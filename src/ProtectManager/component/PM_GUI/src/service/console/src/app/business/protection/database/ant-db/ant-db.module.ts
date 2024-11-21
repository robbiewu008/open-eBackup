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
import { DatabaseTemplateModule } from 'app/business/protection/host-app/database-template/database-template.module';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AntDBRoutingModule } from './ant-db-routing.module';
import { AntDBComponent } from './ant-db.component';
import { RegisterModule } from './register/register.module';
import { RestoreModule } from './restore/restore.module';

@NgModule({
  declarations: [AntDBComponent],
  imports: [
    CommonModule,
    RegisterModule,
    BaseModule,
    RestoreModule,
    AntDBRoutingModule,
    DatabaseTemplateModule,
    ProButtonModule,
    ProTableModule
  ]
})
export class AntDBModule {}
