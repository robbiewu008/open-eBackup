import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { HdfsComponent } from './hdfs.component';

const routes: Routes = [{ path: '', component: HdfsComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class HdfsRoutingModule {}
