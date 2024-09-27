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
import { RouterModule, Routes } from '@angular/router';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { CopyDatabaseComponent } from './copy-database.component';

const routes: Routes = [
  {
    path: '',
    component: CopyDatabaseComponent,
    children: [
      { path: '', redirectTo: 'oracle', pathMatch: 'full' },
      {
        path: 'oracle',
        loadChildren: () =>
          import('../oracle/oracle.module').then(mod => mod.OracleModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'mysql',
        loadChildren: () =>
          import('../mysql/mysql.module').then(mod => mod.MysqlModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'sql-server',
        loadChildren: () =>
          import('../sql-server/sql-server.module').then(
            mod => mod.SQLServerModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'postgre-sql',
        loadChildren: () =>
          import('../postgre-sql/postgre-sql.module').then(
            mod => mod.PostgreSQLModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'db2',
        loadChildren: () =>
          import('../db-two/db-two.module').then(mod => mod.DbTwoModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'informix',
        loadChildren: () =>
          import('../informix/informix.module').then(mod => mod.InformixModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'opengauss',
        loadChildren: () =>
          import('../opengauss/opengauss.module').then(
            mod => mod.OpengaussModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'gaussdb-t',
        loadChildren: () =>
          import('../gaussdb-t/gaussdb-t.module').then(
            mod => mod.GaussdbTModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'tidb',
        loadChildren: () =>
          import('../tidb/tidb.module').then(mod => mod.TidbModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'ocean-base',
        loadChildren: () =>
          import('../ocean-base/ocean-base.module').then(
            mod => mod.OceanBaseModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'tdsql',
        loadChildren: () =>
          import('../tdsql/tdsql.module').then(mod => mod.TdsqlModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'king-base',
        loadChildren: () =>
          import('../king-base/king-base.module').then(
            mod => mod.KingBaseModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'dameng',
        loadChildren: () =>
          import('../dameng/dameng.module').then(mod => mod.DamengModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'goldendb',
        loadChildren: () =>
          import('../goldendb/goldendb.module').then(mod => mod.GoldendbModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'general-database',
        loadChildren: () =>
          import('../general-database/general-database.module').then(
            mod => mod.GeneralDatabaseModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'gbase',
        loadChildren: () =>
          import('../gbase/gbase.module').then(mod => mod.GbaseModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'light-cloud-gaussdb',
        loadChildren: () =>
          import('../light-cloud-gaussdb/light-cloud-gaussdb.module').then(
            mod => mod.LightCloudGaussdbModule
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
export class CopyDatabaseRoutingModule {}
