import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { CnwareComponent } from './cnware.component';

const routes: Routes = [{ path: '', component: CnwareComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CnwareRoutingModule {}
