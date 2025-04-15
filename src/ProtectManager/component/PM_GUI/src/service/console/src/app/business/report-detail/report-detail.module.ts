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
import { ReportDetailRouterModule } from './report-detail.routing';
import { ReportDetailComponent } from './report-detail.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { BaseModule } from 'app/shared';
import { AlertModule } from '@iux/live';
import { CapacityTrendChartModule } from '../explore/detection-report/capacity-trend-chart/capacity-trend-chart.module';
import { TrendChartModule } from '../explore/detection-report/trend-chart/trend-chart.module';

@NgModule({
  imports: [
    CommonModule,
    ReportDetailRouterModule,
    BaseModule,
    ProTableModule,
    ProStatusModule,
    AlertModule,
    TrendChartModule,
    CapacityTrendChartModule
  ],
  declarations: [ReportDetailComponent]
})
export class ReportDetailModule {}
