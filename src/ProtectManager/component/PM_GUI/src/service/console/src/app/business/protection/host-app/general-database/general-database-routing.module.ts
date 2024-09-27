import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { GeneralDatabaseComponent } from './general-database.component';

const routes: Routes = [{ path: '', component: GeneralDatabaseComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class GeneralDatabaseRoutingModule {}
