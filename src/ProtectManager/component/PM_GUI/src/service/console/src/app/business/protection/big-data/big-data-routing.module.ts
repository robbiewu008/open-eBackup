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
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { BigDataComponent } from './big-data.component';

const routes: Routes = [
  {
    path: '',
    component: BigDataComponent,
    children: [
      { path: '', redirectTo: 'mongodb', pathMatch: 'full' },
      {
        path: 'mongodb',
        loadChildren: () =>
          import('../host-app/mongodb/mongodb.module').then(
            mod => mod.MongodbModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'redis',
        loadChildren: () =>
          import('../host-app/redis/redis.module').then(mod => mod.RedisModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'gaussdb-dws',
        loadChildren: () =>
          import('../host-app/gaussdb-dws/gaussdb-dws.module').then(
            mod => mod.GaussdbDWSModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'click-house',
        loadChildren: () =>
          import('../host-app/click-house/click-house.module').then(
            mod => mod.ClickHouseModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'hdfs',
        loadChildren: () =>
          import('./hdfs/hdfs.module').then(mod => mod.HdfsModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'hbase',
        loadChildren: () =>
          import('./hbase/hbase.module').then(mod => mod.HbaseModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'hive',
        loadChildren: () =>
          import('./hive/hive.module').then(mod => mod.HiveModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'elasticsearch',
        loadChildren: () =>
          import('./elasticSearch/elastic-search.module').then(
            mod => mod.ElasticSearchModule
          ),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class BigDataRoutingModule {}
