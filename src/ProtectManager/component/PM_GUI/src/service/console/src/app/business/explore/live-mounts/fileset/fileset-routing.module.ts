import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { FilesetComponent } from './fileset.component';

const routes: Routes = [{ path: '', component: FilesetComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class FilesetRoutingModule {}
