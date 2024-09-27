import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { IbmcConfigComponent } from './ibmc-config.component';

const routes: Routes = [{ path: '', component: IbmcConfigComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class IbmcConfigRoutingModule {}
