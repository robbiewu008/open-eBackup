import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { ObjectComponent } from './object.component';

const routes: Routes = [{ path: '', component: ObjectComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ObjectRoutingModule {}
