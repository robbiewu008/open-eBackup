import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SaphanaComponent } from './saphana.component';

const routes: Routes = [{ path: '', component: SaphanaComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SaphanaRoutingModule {}
