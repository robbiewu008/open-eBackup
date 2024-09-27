import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { GoldendbComponent } from './goldendb.component';

const routes: Routes = [{ path: '', component: GoldendbComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class GoldendbRoutingModule {}
