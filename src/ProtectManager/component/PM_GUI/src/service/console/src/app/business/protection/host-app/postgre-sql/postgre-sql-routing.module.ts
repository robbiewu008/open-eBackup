import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { PostgreSqlComponent } from './postgre-sql.component';

const routes: Routes = [{ path: '', component: PostgreSqlComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class PostgreSqlRoutingModule {}
