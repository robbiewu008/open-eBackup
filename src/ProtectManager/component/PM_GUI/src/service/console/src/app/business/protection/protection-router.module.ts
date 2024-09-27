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
import { ProtectionComponent } from './protection.component';

const routes: Routes = [
  {
    path: '',
    component: ProtectionComponent,
    children: [
      { path: '', redirectTo: 'summary', pathMatch: 'full' },
      {
        path: 'summary',
        loadChildren: () =>
          import('./summary/summary.module').then(mod => mod.SummaryModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'database',
        loadChildren: () =>
          import('./database/database.module').then(mod => mod.DatabaseModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'big-data',
        loadChildren: () =>
          import('./big-data/big-data.module').then(mod => mod.BigDataModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'virtualization',
        loadChildren: () =>
          import('./virtualization/virtualization.module').then(
            mod => mod.VirtualizationModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'container',
        loadChildren: () =>
          import('./container/container.module').then(
            mod => mod.ContainerModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'cloud',
        loadChildren: () =>
          import('./cloud/cloud.module').then(mod => mod.CloudModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'private-cloud',
        loadChildren: () =>
          import('./private-cloud/private-cloud.module').then(
            mod => mod.PrivateCloudModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'file-service',
        loadChildren: () =>
          import('./storage/file-service.module').then(
            mod => mod.FileServiceModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'application',
        loadChildren: () =>
          import('./application/application.module').then(
            mod => mod.ApplicationModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'bare-metal',
        loadChildren: () =>
          import('./bare-metal/bare-metal.module').then(
            mod => mod.BareMetalModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'host',
        loadChildren: () =>
          import('./host-app/host/host.module').then(mod => mod.HostModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'register-host',
        loadChildren: () =>
          import('./host-app/host-register/host-register.module').then(
            mod => mod.HostRegisterModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'storage/local-file-system',
        loadChildren: () =>
          import('./storage/local-file-system/local-file-system.module').then(
            mod => mod.LocalFileSystemModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'storage/local-resource',
        loadChildren: () =>
          import('./storage/local-resource/local-resource.module').then(
            mod => mod.LocalResourceModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'policy/sla',
        loadChildren: () =>
          import('./policy/sla/sla.module').then(mod => mod.SlaModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'policy/limit-rate-policy',
        loadChildren: () =>
          import('./policy/limit-rate-policy/limit-rate-policy.module').then(
            mod => mod.LimitRatePolicyModule
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
export class ProtectionRouterModule {}
