import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DesensitizationPolicyComponent } from './desensitization-policy.component';

const routes: Routes = [
  { path: '', component: DesensitizationPolicyComponent }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DesensitizationPolicyRoutingModule {}
