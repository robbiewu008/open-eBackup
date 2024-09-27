import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { LocalStorageComponent } from './local-storage.component';

const routes: Routes = [{ path: '', component: LocalStorageComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class LocalStorageRoutingModule {}
