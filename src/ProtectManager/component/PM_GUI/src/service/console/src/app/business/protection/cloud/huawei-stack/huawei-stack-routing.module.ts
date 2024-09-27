import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { HuaweiStackComponent } from './huawei-stack.component';

const routes: Routes = [{ path: '', component: HuaweiStackComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class HuaweiStackRoutingModule {}
