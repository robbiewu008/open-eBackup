import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { DamengComponent } from './dameng.component';

const routes: Routes = [{ path: '', component: DamengComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DamengRoutingModule {}
