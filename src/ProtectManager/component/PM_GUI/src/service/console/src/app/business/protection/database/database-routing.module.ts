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
import { DatabaseComponent } from './database.component';

const routes: Routes = [
  {
    path: '',
    component: DatabaseComponent,
    children: [
      { path: '', redirectTo: 'oracle', pathMatch: 'full' },
      {
        path: 'ant-db',
        loadChildren: () =>
          import('./ant-db/ant-db.module').then(mod => mod.AntDBModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'oracle',
        loadChildren: () =>
          import('../host-app/oracle/oracle.module').then(
            mod => mod.OracleModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'mysql',
        loadChildren: () =>
          import('../host-app/mysql/mysql.module').then(mod => mod.MysqlModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'postgre-sql',
        loadChildren: () =>
          import('../host-app/postgre-sql/postgre-sql.module').then(
            mod => mod.PostgreSqlModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'db-two',
        loadChildren: () =>
          import('../host-app/db-two/db-two.module').then(
            mod => mod.DbTwoModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'sql-server',
        loadChildren: () =>
          import('../host-app/sql-server/sql-server.module').then(
            mod => mod.SQLServerModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'dameng',
        loadChildren: () =>
          import('../host-app/dameng/dameng.module').then(
            mod => mod.DamengModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'opengauss',
        loadChildren: () =>
          import('../host-app/opengauss/opengauss.module').then(
            mod => mod.OpengaussModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'gaussdb-t',
        loadChildren: () =>
          import('../host-app/gaussdb-t/gaussdb-t.module').then(
            mod => mod.GaussdbTModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'tidb',
        loadChildren: () =>
          import('../host-app/tidb/tidb.module').then(mod => mod.TidbModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'ocean-base',
        loadChildren: () =>
          import('../host-app/ocean-base/ocean-base.module').then(
            mod => mod.OceanBaseModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'tdsql',
        loadChildren: () =>
          import('../host-app/tdsql/tdsql.module').then(mod => mod.TdsqlModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'goldendb',
        loadChildren: () =>
          import('../host-app/goldendb/goldendb.module').then(
            mod => mod.GoldendbModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'king-base',
        loadChildren: () =>
          import('../host-app/king-base/king-base.module').then(
            mod => mod.KingBaseModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'informix',
        loadChildren: () =>
          import('../host-app/informix/informix.module').then(
            mod => mod.InformixModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'general-database',
        loadChildren: () =>
          import('../host-app/general-database/general-database.module').then(
            mod => mod.GeneralDatabaseModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'gbase',
        loadChildren: () =>
          import('./gbase/gbase.module').then(mod => mod.GbaseModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'light-cloud-gaussdb',
        loadChildren: () =>
          import(
            '../host-app/light-cloud-gaussdb/light-cloud-gaussdb.module'
          ).then(mod => mod.LightCloudGaussdbModule),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DatabaseRoutingModule {}
