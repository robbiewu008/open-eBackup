import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { CyberAlarmNotifyComponent } from './cyber-alarm-notify.component';

const routes: Routes = [{ path: '', component: CyberAlarmNotifyComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CyberAlarmNotifyRoutingModule {}
