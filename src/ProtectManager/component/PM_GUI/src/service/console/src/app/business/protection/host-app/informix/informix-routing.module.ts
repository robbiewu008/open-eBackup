import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { InformixComponent } from './informix.component';

const routes: Routes = [{ path: '', component: InformixComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class InformixRoutingModule {}
