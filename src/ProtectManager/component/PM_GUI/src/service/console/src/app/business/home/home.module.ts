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
import {
  BadgeModule,
  LayoutModule,
  GroupModule,
  ButtonModule
} from '@iux/live';
import { JobChartModule } from 'app/shared/components/charts/job-chart/job-chart.module';
import { ResourceDisplayBadgeModule } from 'app/shared/components/charts/resource-display-badge/resource-display-badge.module';
import { AlarmChartModule } from 'app/shared/components/charts/alarm-chart/alarm-chart.module';
import { SystemCapacityChartModule } from 'app/shared/components/charts/system-capacity-chart/system-capacity-chart.module';
import { BackupCapacityChartModule } from 'app/shared/components/charts/backup-capacity-chart/backup-capacity-chart.module';
import { ArchiveCapacityChartModule } from 'app/shared/components/charts/archive-capacity-chart/archive-capacity-chart.module';
import { CapacityForecastChartModule } from 'app/shared/components/charts/capacity-forecast-chart/capacity-forecast-chart.module';
import { PerformanceChartModule } from 'app/shared/components/charts/performance-chart/performance-chart.module';
import { DataReductionModule } from 'app/shared/components';
import { SlaComplianceChartModule } from 'app/shared/components/charts/sla-compliance-chart/sla-compliance-chart.module';
import { ClusterNodeTabModule } from 'app/shared/components/charts/cluster-node-tab/cluster-node-tab.module';
import { DetectionStatisticsModule } from '../explore/anti-ransomware/detection-statistics/detection-statistics.module';
import { BaseModule } from 'app/shared';
import { CyberEngineHomeModule } from './cyber-engine-home/cyber-engine-home.module';
import { MultiClusterOverviewModule } from 'app/shared/components/charts/multi-cluster-overview/multi-cluster-overview.module';
import { CoupleNodeChartModule } from 'app/shared/components/charts/couple-node-chart/couple-node-chart.module';

const ChartModule = [
  JobChartModule,
  ResourceDisplayBadgeModule,
  AlarmChartModule,
  DataReductionModule,
  SystemCapacityChartModule,
  BackupCapacityChartModule,
  ArchiveCapacityChartModule,
  CapacityForecastChartModule,
  PerformanceChartModule,
  SlaComplianceChartModule,
  ClusterNodeTabModule,
  MultiClusterOverviewModule,
  CyberEngineHomeModule,
  CoupleNodeChartModule
];

@NgModule({
  imports: [
    ...ChartModule,
    HomeRouterModule,
    LayoutModule,
    BadgeModule,
    GroupModule,
    ButtonModule,
    BaseModule,
    DetectionStatisticsModule
  ],
  declarations: [HomeComponent],
  exports: [HomeComponent]
})
export class HomeModule {}
