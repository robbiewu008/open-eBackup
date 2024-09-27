import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { VolumeComponent } from './volume.component';

const routes: Routes = [{ path: '', component: VolumeComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class VolumeRoutingModule {}
