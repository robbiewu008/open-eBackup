import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SftpServiceComponent } from './sftp-service.component';

const routes: Routes = [{ path: '', component: SftpServiceComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SftpServiceRoutingModule {}
