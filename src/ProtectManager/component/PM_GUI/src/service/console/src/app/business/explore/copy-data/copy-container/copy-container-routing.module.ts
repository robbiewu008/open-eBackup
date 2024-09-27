import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { CopyContainerComponent } from './copy-container.component';

const routes: Routes = [
  {
    path: '',
    component: CopyContainerComponent,
    children: [
      {
        path: '',
        redirectTo: 'kubernetes',
        pathMatch: 'full'
      },
      {
        path: 'kubernetes',
        loadChildren: () =>
          import('../kubernetes/kubernetes.module').then(
            mod => mod.KubernetesModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'kubernetes-container',
        loadChildren: () =>
          import('../kubernetes-container/kubernetes-container.module').then(
            mod => mod.KubernetesContainerModule
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
export class CopyContainerRoutingModule {}
