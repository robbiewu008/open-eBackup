import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { LimitRatePolicyComponent } from './limit-rate-policy.component';

const routes: Routes = [{ path: '', component: LimitRatePolicyComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class LimitRatePolicyRoutingModule {}
