import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { DetectionModelListComponent } from './detection-model-list.component';

const routes: Routes = [{ path: '', component: DetectionModelListComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DetectionModelListRoutingModule {}
