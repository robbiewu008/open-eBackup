import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DrillExecuteLogComponent } from './drill-execute-log.component';

const routes: Routes = [{ path: '', component: DrillExecuteLogComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DrillExecuteLogRoutingModule {}
