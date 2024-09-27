import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ClusterManagementComponent } from './cluster-management.component';

const routes: Routes = [{ path: '', component: ClusterManagementComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ClusterManagementRoutingModule {}
