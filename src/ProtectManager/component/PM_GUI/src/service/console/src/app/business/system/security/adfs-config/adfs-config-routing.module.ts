import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { AdfsConfigComponent } from './adfs-config.component';

const routes: Routes = [{ path: '', component: AdfsConfigComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class AdfsConfigRoutingModule {}
