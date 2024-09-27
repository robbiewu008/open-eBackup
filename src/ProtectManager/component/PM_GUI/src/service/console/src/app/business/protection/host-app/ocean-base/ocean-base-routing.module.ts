import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { OceanBaseComponent } from './ocean-base.component';

const routes: Routes = [{ path: '', component: OceanBaseComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class OceanBaseRoutingModule {}
