import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { HyperVComponent } from './hyper-v.component';

const routes: Routes = [{ path: '', component: HyperVComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class HyperVRoutingModule {}
