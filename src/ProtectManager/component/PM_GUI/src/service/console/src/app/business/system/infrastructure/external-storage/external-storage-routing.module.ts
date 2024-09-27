import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ExternalStorageComponent } from './external-storage.component';

const routes: Routes = [{ path: '', component: ExternalStorageComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ExternalStorageRoutingModule {}
