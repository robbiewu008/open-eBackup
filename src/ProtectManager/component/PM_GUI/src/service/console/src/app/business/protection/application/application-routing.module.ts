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
import { ApplicationComponent } from './application.component';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';

const routes: Routes = [
  {
    path: '',
    component: ApplicationComponent,
    children: [
      { path: '', redirectTo: 'active-directory', pathMatch: 'full' },
      {
        path: 'active-directory',
        loadChildren: () =>
          import('./active-directory/active-directory.module').then(
            mod => mod.ActiveDirectoryModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'exchange',
        loadChildren: () =>
          import('../host-app/exchange/exchange.module').then(
            mod => mod.ExchangeModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'sap-hana',
        loadChildren: () =>
          import('./saphana/saphana.module').then(mod => mod.SaphanaModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'sap-on-oracle',
        loadChildren: () =>
          import('./saponoracle/saponoracle.module').then(
            mod => mod.SaponoracleModule
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
export class ApplicationRoutingModule {}
