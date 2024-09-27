import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { ApplicationComponent } from './application.component';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';

const routes: Routes = [
  {
    path: '',
    component: ApplicationComponent,
    children: [
      { path: '', redirectTo: 'fileset', pathMatch: 'full' },
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
      },
      {
        path: 'exchange',
        loadChildren: () =>
          import('../exchange/exchange.module').then(mod => mod.ExchangeModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'oracle',
        loadChildren: () =>
          import('../oracle/oracle.module').then(mod => mod.OracleModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'mysql',
        loadChildren: () =>
          import('../mysql/mysql.module').then(mod => mod.MysqlModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'tdsql',
        loadChildren: () =>
          import('../tdsql/tdsql.module').then(mod => mod.TdsqlModule),
        canActivateChild: [RedirectGuard]
      },
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
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ApplicationRoutingModule {}
