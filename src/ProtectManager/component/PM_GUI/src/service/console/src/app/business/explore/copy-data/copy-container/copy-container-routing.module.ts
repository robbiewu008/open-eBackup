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
import { CopyContainerComponent } from './copy-container.component';

const routes: Routes = [
  {
    path: '',
    component: CopyContainerComponent,
    children: [
      {
        path: '',
        redirectTo: 'kubernetes',
        pathMatch: 'full'
      },
      {
        path: 'kubernetes',
        loadChildren: () =>
          import('../kubernetes/kubernetes.module').then(
            mod => mod.KubernetesModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'kubernetes-container',
        loadChildren: () =>
          import('../kubernetes-container/kubernetes-container.module').then(
            mod => mod.KubernetesContainerModule
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
export class CopyContainerRoutingModule {}
