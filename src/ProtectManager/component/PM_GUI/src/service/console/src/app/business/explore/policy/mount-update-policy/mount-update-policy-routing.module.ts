import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { MountUpdatePolicyComponent } from './mount-update-policy.component';

const routes: Routes = [{ path: '', component: MountUpdatePolicyComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class MountUpdatePolicyRoutingModule {}
