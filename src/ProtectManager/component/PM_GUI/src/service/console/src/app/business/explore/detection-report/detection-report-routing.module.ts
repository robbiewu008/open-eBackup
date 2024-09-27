import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DetectionReportComponent } from './detection-report.component';

const routes: Routes = [{ path: '', component: DetectionReportComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DetectionReportRoutingModule {}
