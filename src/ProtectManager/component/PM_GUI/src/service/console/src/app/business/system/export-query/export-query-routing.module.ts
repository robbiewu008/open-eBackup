import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ExportQueryComponent } from './export-query.component';

const routes: Routes = [{ path: '', component: ExportQueryComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ExportQueryRoutingModule {}
