import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { StorageDeviceComponent } from './storage-device.component';

const routes: Routes = [{ path: '', component: StorageDeviceComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class StorageDeviceRoutingModule {}
