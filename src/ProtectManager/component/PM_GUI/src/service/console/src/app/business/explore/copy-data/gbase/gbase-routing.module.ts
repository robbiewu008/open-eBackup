import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { GbaseComponent } from './gbase.component';

const routes: Routes = [{ path: '', component: GbaseComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class GbaseRoutingModule {}
