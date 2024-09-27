import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SamlSsoConfigComponent } from './saml-sso-config.component';

const routes: Routes = [{ path: '', component: SamlSsoConfigComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SamlSsoConfigRoutingModule {}
