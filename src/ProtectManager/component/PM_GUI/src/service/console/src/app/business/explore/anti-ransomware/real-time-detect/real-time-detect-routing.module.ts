import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { RealTimeDetectComponent } from './real-time-detect.component';

const routes: Routes = [{ path: '', component: RealTimeDetectComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class RealTimeDetectRoutingModule {}
