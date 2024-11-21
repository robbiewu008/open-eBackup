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
import { CopyVirtualizationComponent } from './copy-virtualization.component';

const routes: Routes = [
  {
    path: '',
    component: CopyVirtualizationComponent,
    children: [
      { path: '', redirectTo: 'vmware', pathMatch: 'full' },
      {
        path: 'vmware',
        loadChildren: () =>
          import('../vmware/vmware.module').then(mod => mod.VmwareModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'cnware',
        loadChildren: () =>
          import('../cnware/cnware.module').then(mod => mod.CnwareModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'fusion-compute',
        loadChildren: () =>
          import('../fusion-compute/fusion-compute.module').then(
            mod => mod.FusionComputeModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'hyper-v',
        loadChildren: () =>
          import('../hyper-v/hyper-v.module').then(mod => mod.HyperVModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'fusion-one',
        loadChildren: () =>
          import('../fusion-one/fusion-one.module').then(
            mod => mod.FusionOneModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'nutanix',
        loadChildren: () =>
          import('../nutanix/nutanix.module').then(mod => mod.NutanixModule),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CopyVirtualizationRoutingModule {}
