import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { FusionOneComponent } from './fusion-one.component';

const routes: Routes = [{ path: '', component: FusionOneComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class FusionOneRoutingModule {}
