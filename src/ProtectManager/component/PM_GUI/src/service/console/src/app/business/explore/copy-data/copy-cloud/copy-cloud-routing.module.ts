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
import { CopyCloudComponent } from './copy-cloud.component';

const routes: Routes = [
  {
    path: '',
    component: CopyCloudComponent,
    children: [
      { path: '', redirectTo: 'huawei-stack', pathMatch: 'full' },
      {
        path: 'huawei-stack',
        loadChildren: () =>
          import('../huawei-stack/huawei-stack.module').then(
            mod => mod.HuaweiStackModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'openstack',
        loadChildren: () =>
          import('../openstack/openstack.module').then(
            mod => mod.OpenstackModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'gaussdb-for-opengauss',
        loadChildren: () =>
          import('../gaussdb-for-opengauss/gaussdb-for-opengauss.module').then(
            mod => mod.GaussdbForOpengaussModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'apsara-stack',
        loadChildren: () =>
          import('../apsara-stack/apsara-stack.module').then(
            mod => mod.ApsaraStackModule
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
export class CopyCloudRoutingModule {}
