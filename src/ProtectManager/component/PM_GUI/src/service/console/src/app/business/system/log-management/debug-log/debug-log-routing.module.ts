import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DebugLogComponent } from './debug-log.component';

const routes: Routes = [{ path: '', component: DebugLogComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DebugLogRoutingModule {}
