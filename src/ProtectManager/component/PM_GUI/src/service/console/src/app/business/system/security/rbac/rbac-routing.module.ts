import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { RbacComponent } from './rbac.component';

const routes: Routes = [{ path: '', component: RbacComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class RbacRoutingModule {}
