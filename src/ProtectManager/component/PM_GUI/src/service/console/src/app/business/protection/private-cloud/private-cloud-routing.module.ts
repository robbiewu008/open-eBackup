import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { PrivateCloudComponent } from './private-cloud.component';

const routes: Routes = [{ path: '', component: PrivateCloudComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class PrivateCloudRoutingModule {}
