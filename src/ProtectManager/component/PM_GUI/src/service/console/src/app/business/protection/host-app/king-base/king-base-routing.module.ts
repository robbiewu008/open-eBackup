import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { KingBaseComponent } from './king-base.component';

const routes: Routes = [{ path: '', component: KingBaseComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class KingBaseRoutingModule {}
