import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { ConfigNetworkComponent } from './config-network.component';

const routes: Routes = [{ path: '', component: ConfigNetworkComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ConfigNetworkRoutingModule {}
