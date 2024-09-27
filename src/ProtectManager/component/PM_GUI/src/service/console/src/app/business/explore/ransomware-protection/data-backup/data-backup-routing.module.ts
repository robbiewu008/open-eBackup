import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DataBackupComponent } from './data-backup.component';

const routes: Routes = [{ path: '', component: DataBackupComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DataBackupRoutingModule {}
