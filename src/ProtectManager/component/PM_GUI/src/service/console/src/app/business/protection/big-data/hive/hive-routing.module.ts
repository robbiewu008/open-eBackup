import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { HiveComponent } from './hive.component';

const routes: Routes = [{ path: '', component: HiveComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class HiveRoutingModule {}
