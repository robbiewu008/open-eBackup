import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { OpengaussComponent } from './opengauss.component';

const routes: Routes = [{ path: '', component: OpengaussComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class OpengaussRoutingModule {}
