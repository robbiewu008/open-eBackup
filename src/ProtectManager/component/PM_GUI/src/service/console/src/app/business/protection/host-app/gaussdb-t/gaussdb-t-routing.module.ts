import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { GaussdbTComponent } from './gaussdb-t.component';

const routes: Routes = [{ path: '', component: GaussdbTComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class GaussdbTRoutingModule {}
