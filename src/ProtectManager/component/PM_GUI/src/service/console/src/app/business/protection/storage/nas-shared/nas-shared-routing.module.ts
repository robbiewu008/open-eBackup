import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { NasSharedComponent } from './nas-shared.component';

const routes: Routes = [{ path: '', component: NasSharedComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class NasSharedRoutingModule {}
