import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { RealTimeDetectionComponent } from './real-time-detection.component';

const routes: Routes = [{ path: '', component: RealTimeDetectionComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class RealTimeDetectionRoutingModule {}
