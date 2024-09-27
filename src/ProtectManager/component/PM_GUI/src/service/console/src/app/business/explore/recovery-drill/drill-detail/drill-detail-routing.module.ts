import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DrillDetailComponent } from './drill-detail.component';

const routes: Routes = [{ path: '', component: DrillDetailComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DrillDetailRoutingModule {}
