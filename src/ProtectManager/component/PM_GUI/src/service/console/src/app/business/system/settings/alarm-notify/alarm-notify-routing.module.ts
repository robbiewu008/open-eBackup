import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { AlarmNotifyComponent } from './alarm-notify.component';

const routes: Routes = [{ path: '', component: AlarmNotifyComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class AlarmNotifyRoutingModule {}
