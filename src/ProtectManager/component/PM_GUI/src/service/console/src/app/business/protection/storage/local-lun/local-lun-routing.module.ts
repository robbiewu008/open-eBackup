import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { LocalLunComponent } from './local-lun.component';

const routes: Routes = [{ path: '', component: LocalLunComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class LocalLunRoutingModule {}
