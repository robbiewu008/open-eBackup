import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DetectionModelComponent } from './detection-model.component';

const routes: Routes = [{ path: '', component: DetectionModelComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DetectionModelRoutingModule {}
