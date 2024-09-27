import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { FileServiceComponent } from './file-service.component';

const routes: Routes = [
  {
    path: '',
    component: FileServiceComponent,
    children: [
      { path: '', redirectTo: 'storage-device-info', pathMatch: 'full' },
      {
        path: 'storage-device-info',
        loadChildren: () =>
          import('./storage-device-info/storage-device-info.module').then(
            mod => mod.StorageDeviceInfoModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'dorado-file-system',
        loadChildren: () =>
          import('./dorado-file-system/dorado-file-system.module').then(
            mod => mod.DoradoFileSystemModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'nas-shared',
        loadChildren: () =>
          import('./nas-shared/nas-shared.module').then(
            mod => mod.NasSharedModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'common-share',
        loadChildren: () =>
          import('./commonshare/commonshare.module').then(
            mod => mod.CommonShareModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'object',
        loadChildren: () =>
          import('./object/object.module').then(mod => mod.ObjectModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'fileset-template',
        loadChildren: () =>
          import(
            '../host-app/fileset/fileset-template-list/fileset-template-list.module'
          ).then(mod => mod.FilesetTemplateListModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'volume',
        loadChildren: () =>
          import('../host-app/volume/volume.module').then(
            mod => mod.VolumeModule
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
export class FileServiceRoutingModule {}
