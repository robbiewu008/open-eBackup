import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { RecoveryDrillComponent } from './recovery-drill.component';

const routes: Routes = [{ path: '', component: RecoveryDrillComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class RecoveryDrillRoutingModule {}
