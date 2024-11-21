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

import { MongodbRoutingModule } from './mongodb-routing.module';
import { MongodbComponent } from './mongodb.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { BaseTemplateModule } from '../../virtualization/kubernetes/base-template/base-template.module';
import { BaseModule } from 'app/shared';
import { RegisterMongodbModule } from './register-mongodb/register-mongodb.module';
import { SummaryModule } from './summary/summary.module';
import { CopyDataModule } from './copy-data/copy-data.module';

@NgModule({
  declarations: [MongodbComponent],
  imports: [
    CommonModule,
    BaseModule,
    MongodbRoutingModule,
    MultiClusterSwitchModule,
    BaseTemplateModule,
    RegisterMongodbModule,
    SummaryModule,
    CopyDataModule
  ]
})
export class MongodbModule {}
