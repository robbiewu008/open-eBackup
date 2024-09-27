import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { GaussdbForOpengaussComponent } from './gaussdb-for-opengauss.component';

const routes: Routes = [{ path: '', component: GaussdbForOpengaussComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class GaussdbForOpengaussRoutingModule {}
