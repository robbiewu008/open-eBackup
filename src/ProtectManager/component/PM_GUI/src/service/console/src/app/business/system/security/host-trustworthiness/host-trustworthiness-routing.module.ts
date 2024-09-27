import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { HostTrustworthinessComponent } from './host-trustworthiness.component';

const routes: Routes = [{ path: '', component: HostTrustworthinessComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class HostTrustworthinessRoutingModule {}
