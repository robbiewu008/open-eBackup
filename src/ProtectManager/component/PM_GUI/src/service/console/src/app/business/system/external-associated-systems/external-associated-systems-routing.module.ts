import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ExternalAssociatedSystemsComponent } from './external-associated-systems.component';

const routes: Routes = [
  { path: '', component: ExternalAssociatedSystemsComponent }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ExternalAssociatedSystemsRoutingModule {}
