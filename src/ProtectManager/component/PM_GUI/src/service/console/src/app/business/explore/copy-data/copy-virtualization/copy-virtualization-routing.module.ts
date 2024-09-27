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
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CopyVirtualizationRoutingModule {}
