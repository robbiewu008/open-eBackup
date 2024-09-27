import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { AlarmSettingsComponent } from './alarm-settings.component';

const routes: Routes = [{ path: '', component: AlarmSettingsComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class AlarmSettingsRoutingModule {}
