import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { ApsaraStackComponent } from './apsara-stack.component';

const routes: Routes = [{ path: '', component: ApsaraStackComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ApsaraStackRoutingModule {}
