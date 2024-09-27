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
import { ActiveDirectoryComponent } from './active-directory.component';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../../host-app/database-template/database-template.module';
import { ActiveDirectoryRoutingModule } from './active-directory-routing.module';
import { RegisterModule } from './register/register.module';
import { SummaryModule } from './summary/summary.module';
import { RestoreModule } from './restore/restore.module';

@NgModule({
  declarations: [ActiveDirectoryComponent],
  imports: [
    CommonModule,
    BaseModule,
    DatabaseTemplateModule,
    ActiveDirectoryRoutingModule,
    RegisterModule,
    SummaryModule,
    RestoreModule
  ]
})
export class ActiveDirectoryModule {}
