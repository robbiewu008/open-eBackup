import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { SystemTimeComponent } from './system-time.component';

const routes: Routes = [{ path: '', component: SystemTimeComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SystemTimeRoutingModule {}
