import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SnapshotDataComponent } from './snapshot-data.component';

const routes: Routes = [{ path: '', component: SnapshotDataComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SnapshotDataRoutingModule {}
