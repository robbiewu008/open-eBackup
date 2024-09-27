import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ClickHouseComponent } from './click-house.component';
const routes: Routes = [{ path: '', component: ClickHouseComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ClickHouseRoutingModule {}
