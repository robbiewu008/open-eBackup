import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { CloudBackupOverviewComponent } from './cloud-backup-overview.component';

const routes: Routes = [{ path: '', component: CloudBackupOverviewComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CloudBackupOverviewRoutingModule {}
