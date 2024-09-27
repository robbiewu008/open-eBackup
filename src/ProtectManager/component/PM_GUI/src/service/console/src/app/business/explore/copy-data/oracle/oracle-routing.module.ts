import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { OracleComponent } from './oracle.component';

const routes: Routes = [{ path: '', component: OracleComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class OracleRoutingModule {}
