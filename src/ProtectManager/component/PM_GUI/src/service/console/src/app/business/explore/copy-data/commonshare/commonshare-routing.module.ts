import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { CommonshareComponent } from './commonshare.component';

const routes: Routes = [{ path: '', component: CommonshareComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CommonshareRoutingModule {}
