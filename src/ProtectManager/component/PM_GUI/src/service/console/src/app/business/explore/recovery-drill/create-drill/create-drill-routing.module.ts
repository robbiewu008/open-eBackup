import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { CreateDrillComponent } from './create-drill.component';

const routes: Routes = [{ path: '', component: CreateDrillComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CreateDrillRoutingModule {}
