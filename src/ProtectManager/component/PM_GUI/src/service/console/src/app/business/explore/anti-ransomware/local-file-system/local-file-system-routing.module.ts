import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { LocalFileSystemComponent } from './local-file-system.component';

const routes: Routes = [{ path: '', component: LocalFileSystemComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class LocalFileSystemRoutingModule {}
