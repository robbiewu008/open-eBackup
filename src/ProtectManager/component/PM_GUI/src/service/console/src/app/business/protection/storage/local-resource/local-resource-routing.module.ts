import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { LocalResourceComponent } from './local-resource.component';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';

const routes: Routes = [{ path: '', component: LocalResourceComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class LocalResourceRoutingModule {}
