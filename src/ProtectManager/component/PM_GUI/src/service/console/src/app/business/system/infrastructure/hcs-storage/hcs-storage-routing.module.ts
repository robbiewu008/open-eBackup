import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { HcsStorageComponent } from './hcs-storage.component';

const routes: Routes = [{ path: '', component: HcsStorageComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class HcsStorageRoutingModule {}
