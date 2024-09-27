import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { SecuritypolicyComponent } from './security-policy.component';

const routes: Routes = [{ path: '', component: SecuritypolicyComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SecuritypolicyRoutingModule {}
