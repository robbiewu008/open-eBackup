import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { KubernetesContainerComponent } from './kubernetes-container.component';

const routes: Routes = [{ path: '', component: KubernetesContainerComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class KubernetesContainerRoutingModule {}
