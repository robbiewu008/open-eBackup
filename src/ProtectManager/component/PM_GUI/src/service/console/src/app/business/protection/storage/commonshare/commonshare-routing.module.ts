import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { CommonShareComponent } from './commonshare.component';

const routes: Routes = [{ path: '', component: CommonShareComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CommonShareRoutingModule {}
