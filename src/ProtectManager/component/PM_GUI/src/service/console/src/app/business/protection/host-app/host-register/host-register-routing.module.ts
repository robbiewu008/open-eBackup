import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { HostRegisterComponent } from './host-register.component';

const routes: Routes = [{ path: '', component: HostRegisterComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class HostRegisterRoutingModule {}
