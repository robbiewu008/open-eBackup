import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { NetworkConfigComponent } from './network-config.component';

const routes: Routes = [{ path: '', component: NetworkConfigComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class NetworkConfigRoutingModule {}
