import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { PostgreSQLComponent } from './postgre-sql.component';

const routes: Routes = [{ path: '', component: PostgreSQLComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class PostgreSQLRoutingModule {}
