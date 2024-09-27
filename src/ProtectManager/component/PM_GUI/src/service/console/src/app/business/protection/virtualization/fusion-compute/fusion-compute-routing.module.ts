import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { FusionComputeComponent } from './fusion-compute.component';

const routes: Routes = [{ path: '', component: FusionComputeComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class FusionComputeRoutingModule {}
