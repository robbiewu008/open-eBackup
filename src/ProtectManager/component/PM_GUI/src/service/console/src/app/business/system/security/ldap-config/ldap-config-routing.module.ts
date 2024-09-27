import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { LdapConfigComponent } from './ldap-config.component';

const routes: Routes = [{ path: '', component: LdapConfigComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class LdapConfigRoutingModule {}
