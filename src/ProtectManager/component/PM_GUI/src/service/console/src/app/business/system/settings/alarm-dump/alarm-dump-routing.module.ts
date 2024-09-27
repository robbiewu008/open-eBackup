import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { AlarmDumpComponent } from './alarm-dump.component';

const routes: Routes = [{ path: '', component: AlarmDumpComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class AlarmDumpRoutingModule {}
