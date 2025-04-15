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
import { DatabaseTemplateModule } from '../../host-app/database-template/database-template.module';
import { ObjectRoutingModule } from './object-routing.module';
import { ObjectRestoreModule } from './object-service/object-restore/object-restore.module';
import { RegisterObjectModule } from './object-service/register-object/register-object.module';
import { SummaryModule } from './object-service/summary/summary.module';
import { ObjectStorageModule } from './object-storage/object-storage.module';
import { ObjectComponent } from './object.component';

@NgModule({
  declarations: [ObjectComponent],
  imports: [
    CommonModule,
    ObjectRoutingModule,
    BaseModule,
    ObjectStorageModule,
    DatabaseTemplateModule,
    RegisterObjectModule,
    SummaryModule,
    ObjectRestoreModule
  ],
  exports: [ObjectComponent]
})
export class ObjectModule {}
