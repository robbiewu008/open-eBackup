import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { CloudComponent } from './cloud.component';

const routes: Routes = [
  {
    path: '',
    component: CloudComponent,
    children: [
      { path: '', redirectTo: 'huawei-stack', pathMatch: 'full' },
      {
        path: 'huawei-stack',
        loadChildren: () =>
          import('./huawei-stack/huawei-stack.module').then(
            mod => mod.HuaweiStackModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'openstack',
        loadChildren: () =>
          import('./openstack/openstack.module').then(
            mod => mod.OpenstackModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'gaussdb-for-opengauss',
        loadChildren: () =>
          import(
            '../host-app/gaussdb-for-opengauss/gaussdb-for-opengauss.module'
          ).then(mod => mod.GaussdbForOpengaussModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'apsara-stack',
        loadChildren: () =>
          import('./apsara-stack/apsara-stack.module').then(
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
export class CloudRoutingModule {}
