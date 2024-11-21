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
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';
import { FilterTagsModule } from 'app/shared/components/filter-tags/filter-tags.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { BatchOperateServiceModule } from 'app/shared/services/batch-operate.service';
import { BaseModule } from '../../../shared/base.module';
import { AlarmsClearComponent } from './alarms-clear/alarms-clear.component';
import { AlarmsDetailsComponent } from './alarms-details/alarms-details.component';
import { AlarmsRoutingModule } from './alarms-routing.module';
import { AlarmsComponent, SelectionPipe } from './alarms.component';

@NgModule({
  declarations: [
    AlarmsComponent,
    AlarmsDetailsComponent,
    AlarmsClearComponent,
    SelectionPipe
  ],
  imports: [
    BaseModule,
    AlarmsRoutingModule,
    BatchOperateServiceModule,
    MultiClusterSwitchModule,
    CustomTableSearchModule,
    FilterTagsModule
  ]
})
export class AlarmsModule {}
