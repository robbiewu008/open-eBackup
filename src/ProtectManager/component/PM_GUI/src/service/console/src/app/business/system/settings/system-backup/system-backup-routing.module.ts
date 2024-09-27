import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { SystemBackupComponent } from './system-backup.component';

const routes: Routes = [{ path: '', component: SystemBackupComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SystemBackupRoutingModule {}
