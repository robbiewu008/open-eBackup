import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DataSecurityComponent } from './data-security.component';

const routes: Routes = [{ path: '', component: DataSecurityComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DataSecurityRoutingModule {}
