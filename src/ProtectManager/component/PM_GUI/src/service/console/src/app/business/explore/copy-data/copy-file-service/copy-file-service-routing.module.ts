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
import { CopyFileServiceComponent } from './copy-file-service.component';

const routes: Routes = [
  {
    path: '',
    component: CopyFileServiceComponent,
    children: [
      { path: '', redirectTo: 'nas-shared', pathMatch: 'full' },
      {
        path: 'dorado-file-system',
        loadChildren: () =>
          import('../dorado-file-system/dorado-file-system.module').then(
            mod => mod.DoradoFileSystemModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'nas-shared',
        loadChildren: () =>
          import('../nas-shared/nas-shared.module').then(
            mod => mod.NasSharedModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'ndmp',
        loadChildren: () =>
          import('../ndmp/ndmp.module').then(mod => mod.NdmpModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'common-share',
        loadChildren: () =>
          import('../commonshare/commonshare.module').then(
            mod => mod.CommonshareModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'object',
        loadChildren: () =>
          import('../object/object.module').then(mod => mod.ObjectModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'fileset',
        loadChildren: () =>
          import('../fileset/fileset.module').then(mod => mod.FilesetModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'volume',
        loadChildren: () =>
          import('../volume/volume.module').then(mod => mod.VolumeModule),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CopyFileServiceRoutingModule {}
