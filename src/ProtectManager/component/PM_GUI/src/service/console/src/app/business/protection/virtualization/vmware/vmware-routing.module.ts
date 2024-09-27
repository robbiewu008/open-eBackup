import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { VmwareComponent } from './vmware.component';

const routes: Routes = [{ path: '', component: VmwareComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class VmwareRoutingModule {}
