import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { TagManagementComponent } from './tag-management.component';

const routes: Routes = [{ path: '', component: TagManagementComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class TagManagementRoutingModule {}
