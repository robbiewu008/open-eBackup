import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { CopyPrivateCloudComponent } from './copy-private-cloud.component';

const routes: Routes = [{ path: '', component: CopyPrivateCloudComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CopyPrivateCloudRoutingModule {}
