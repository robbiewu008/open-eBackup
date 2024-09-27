import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ArchiveStorageComponent } from './archive-storage.component';

const routes: Routes = [{ path: '', component: ArchiveStorageComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class StorageManagementRoutingModule {}
