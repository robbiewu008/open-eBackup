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
import { NgModule, inject } from '@angular/core';
import { PreloadAllModules, RouterModule, Routes } from '@angular/router';
import { AuthGuard } from './shared/guards/auth.guard';
import { InitResolver } from './shared/guards/init-resolver';
import { LoginGuard } from './shared/guards/login.guard';
import { RedirectGuard } from './shared/guards/redirect.guard';
import { InitMutli } from './shared/guards/init-mutli';
import { InitLicense } from './shared/guards/init-license';
import { PermissionService } from './shared/guards/permission.service';

const routes: Routes = [
  { path: '', redirectTo: '/login', pathMatch: 'full' },
  {
    path: 'login',
    loadChildren: () =>
      import('./business/login/login.module').then(mod => mod.LoginModule),
    canActivate: [LoginGuard]
  },
  {
    path: 'reset-pwd',
    loadChildren: () =>
      import('./business/reset-pwd/reset-pwd.module').then(
        mod => mod.ResetPwdModule
      ),
    canActivate: [LoginGuard]
  },
  {
    path: 'error-page',
    loadChildren: () =>
      import('./business/error-page/error-page.module').then(
        mod => mod.ErrorPageModule
      )
  },
  {
    path: 'report-detail/:id',
    loadChildren: () =>
      import('./business/report-detail/report-detail.module').then(
        mod => mod.ReportDetailModule
      ),
    canActivate: [RedirectGuard, AuthGuard],
    canActivateChild: [RedirectGuard, AuthGuard]
  },
  {
    path: 'init',
    loadChildren: () =>
      import('./business/init/init.module').then(mod => mod.InitModule),
    resolve: {
      init: InitResolver
    },
    canActivate: [RedirectGuard, AuthGuard],
    canActivateChild: [RedirectGuard, AuthGuard]
  },
  {
    path: 'home2',
    loadChildren: () =>
      import('./business/home/home.module').then(mod => mod.HomeModule),
    resolve: {
      init: InitResolver,
      multi: InitMutli,
      license: InitLicense,
      permission: () => inject(PermissionService).getUserPermission()
    },
    canActivate: [RedirectGuard, AuthGuard],
    canActivateChild: [RedirectGuard, AuthGuard]
  },
  {
    path: 'home',
    loadChildren: () =>
      import('./business/home2/home.module').then(mod => mod.HomeModule),
    resolve: {
      init: InitResolver,
      multi: InitMutli,
      license: InitLicense
    },
    canActivate: [RedirectGuard, AuthGuard],
    canActivateChild: [RedirectGuard, AuthGuard]
  },
  {
    path: 'protection',
    loadChildren: () =>
      import('./business/protection/protection.module').then(
        mod => mod.ProtectionModule
      ),
    resolve: {
      init: InitResolver,
      multi: InitMutli,
      license: InitLicense,
      permission: () => inject(PermissionService).getUserPermission()
    },
    canActivate: [RedirectGuard, AuthGuard],
    canActivateChild: [RedirectGuard, AuthGuard]
  },
  {
    path: 'system',
    loadChildren: () =>
      import('./business/system/system.module').then(mod => mod.SystemModule),
    resolve: {
      init: InitResolver,
      multi: InitMutli,
      permission: () => inject(PermissionService).getUserPermission()
    },
    canActivate: [RedirectGuard, AuthGuard],
    canActivateChild: [RedirectGuard, AuthGuard]
  },
  {
    path: 'search',
    loadChildren: () =>
      import('./business/search/search.module').then(
        mod => mod.GlobalSearchModule
      ),
    resolve: {
      init: InitResolver,
      multi: InitMutli,
      permission: () => inject(PermissionService).getUserPermission()
    },
    canActivate: [RedirectGuard, AuthGuard],
    canActivateChild: [RedirectGuard, AuthGuard]
  },
  {
    path: 'insight',
    loadChildren: () =>
      import('./business/insight/insight.module').then(
        mod => mod.InsightModule
      ),
    resolve: {
      init: InitResolver,
      multi: InitMutli,
      license: InitLicense,
      permission: () => inject(PermissionService).getUserPermission()
    },
    canActivate: [RedirectGuard, AuthGuard],
    canActivateChild: [RedirectGuard, AuthGuard]
  },
  {
    path: 'explore',
    loadChildren: () =>
      import('./business/explore/explore.module').then(
        mod => mod.ExploreModule
      ),
    resolve: {
      init: InitResolver,
      multi: InitMutli,
      license: InitLicense,
      permission: () => inject(PermissionService).getUserPermission()
    },
    canActivate: [RedirectGuard, AuthGuard],
    canActivateChild: [RedirectGuard, AuthGuard]
  }
];

@NgModule({
  imports: [
    RouterModule.forRoot(routes, {
      onSameUrlNavigation: 'reload'
    })
  ],
  exports: [RouterModule]
})
export class AppRouterModule {}
