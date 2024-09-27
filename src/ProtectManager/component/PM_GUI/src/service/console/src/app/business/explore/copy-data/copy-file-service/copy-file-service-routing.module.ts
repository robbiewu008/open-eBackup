import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { CopyFileServiceComponent } from './copy-file-service.component';

const routes: Routes = [
  {
    path: '',
    component: CopyFileServiceComponent,
    children: [
      { path: '', redirectTo: 'dorado-file-system', pathMatch: 'full' },
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
