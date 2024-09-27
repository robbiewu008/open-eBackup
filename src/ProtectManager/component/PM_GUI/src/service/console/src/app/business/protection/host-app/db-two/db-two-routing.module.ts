import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DbTwoComponent } from './db-two.component';

const routes: Routes = [{ path: '', component: DbTwoComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DbTwoRoutingModule {}
