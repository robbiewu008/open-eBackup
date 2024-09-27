import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { ContainerComponent } from './container.component';

const routes: Routes = [
  {
    path: '',
    component: ContainerComponent,
    children: [
      { path: '', redirectTo: 'kubernetes', pathMatch: 'full' },
      {
        path: 'kubernetes',
        loadChildren: () =>
          import('../virtualization/kubernetes/kubernetes.module').then(
            mod => mod.KubernetesModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'kubernetes-container',
        loadChildren: () =>
          import(
            '../virtualization/kubernetes-container/kubernetes-container.module'
          ).then(mod => mod.KubernetesContainerModule),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ContainerRoutingModule {}
