import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DistributedNasListComponent } from './distributed-nas-list.component';

const routes: Routes = [{ path: '', component: DistributedNasListComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DistributedNasListRoutingModule {}
