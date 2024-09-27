import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { OpenstackComponent } from './openstack.component';

const routes: Routes = [{ path: '', component: OpenstackComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class OpenstackRoutingModule {}
