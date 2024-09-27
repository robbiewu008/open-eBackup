import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { UserroleComponent } from './user-role.component';

const routes: Routes = [{ path: '', component: UserroleComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class UserroleRoutingModule {}
