import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ReportDetailComponent } from './report-detail.component';

const routes: Routes = [{ path: '', component: ReportDetailComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ReportDetailRouterModule {}
