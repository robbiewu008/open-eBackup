import { GaussDBDWSComponent } from './gaussdb-dws.component';
import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

const routes: Routes = [{ path: '', component: GaussDBDWSComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class GaussDBDWSRoutingModule {}
