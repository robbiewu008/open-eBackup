import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SQLServerComponent } from './sql-server.component';

const routes: Routes = [{ path: '', component: SQLServerComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SQLServerRoutingModule {}
