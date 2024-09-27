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
import { HomeComponent } from './home.component';
import { HomeRouterModule } from './home-router.module';
import { CommonModule } from '@angular/common';
import { MissionOverviewComponent } from './card-components/mission-overview/mission-overview.component';
import { CapacityModule } from './card-components/capacity/capacity.module';
import { CardModule } from './card/card.module';
import { BaseModule } from 'app/shared';
import { AlarmComponent } from './card-components/alarm/alarm.component';
import { ProtectionSituationModule } from './card-components/protection-situation/protection-situation.module';
import { AntiRansomwareComponent } from './card-components/anti-ransomware/anti-ransomware.component';
import { ReductionRateComponent } from './card-components/reduction-rate/reduction-rate.component';
import { ResourceAccessComponent } from './card-components/resource-access/resource-access.component';
import { SlaComplianceModule } from './card-components/sla-compliance/sla-compliance.module';
import { TopFailedTasksResourceObjectsComponent } from './card-components/top-failed-tasks-resource-objects/top-failed-tasks-resource-objects.component';
import { TopFailedTasksSlaProtectionPolicyComponent } from './card-components/top-failed-tasks-sla-protection-policy/top-failed-tasks-sla-protection-policy.component';
import { PerformanceModule } from './card-components/performance/performance.module';
import { CapacityDictionModule } from './card-components/capacity-diction/capacity-diction.module';
import { HomeModule as OldHomeModule } from '../home/home.module';

@NgModule({
  imports: [
    HomeRouterModule,
    CommonModule,
    CardModule,
    BaseModule,
    CapacityModule,
    ProtectionSituationModule,
    SlaComplianceModule,
    PerformanceModule,
    CapacityDictionModule,
    OldHomeModule
  ],
  declarations: [
    HomeComponent,
    MissionOverviewComponent,
    AlarmComponent,
    AntiRansomwareComponent,
    ReductionRateComponent,
    ResourceAccessComponent,
    TopFailedTasksResourceObjectsComponent,
    TopFailedTasksSlaProtectionPolicyComponent
  ]
})
export class HomeModule {}
