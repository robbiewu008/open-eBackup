import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { AirgapComponent } from './airgap.component';

const routes: Routes = [{ path: '', component: AirgapComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class AirgapRoutingModule {}
