import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { GaussdbDWSComponent } from './gaussdb-dws.component';

const routes: Routes = [{ path: '', component: GaussdbDWSComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class GaussdbDWSRoutingModule {}
