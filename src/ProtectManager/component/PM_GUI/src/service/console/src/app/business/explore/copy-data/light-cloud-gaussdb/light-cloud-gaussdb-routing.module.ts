import { LightCloudGaussdbComponent } from './light-cloud-gaussdb.component';
import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

const routes: Routes = [{ path: '', component: LightCloudGaussdbComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class LightCloudGaussdbRoutingModule {}
