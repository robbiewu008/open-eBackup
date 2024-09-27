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
import { Routes, RouterModule } from '@angular/router';
import { InsightComponent } from './insight.component';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';

const routes: Routes = [
  {
    path: '',
    component: InsightComponent,
    children: [
      { path: '', redirectTo: 'insight/performance', pathMatch: 'full' },
      {
        path: 'performance',
        loadChildren: () =>
          import('./performance/performance.module').then(
            mod => mod.PerformanceModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'alarms',
        loadChildren: () =>
          import('./alarms/alarms.module').then(mod => mod.AlarmsModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'jobs',
        loadChildren: () =>
          import('./job/job.module').then(mod => mod.JobModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'reports',
        loadChildren: () =>
          import('./reports/reports.module').then(mod => mod.ReportsModule),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class InsightRoutingModule {}
