import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DetectionSettingComponent } from './detection-setting.component';

const routes: Routes = [{ path: '', component: DetectionSettingComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DetectionSettingRoutingModule {}
