import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { DoradoFileSystemComponent } from './dorado-file-system.component';

const routes: Routes = [{ path: '', component: DoradoFileSystemComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class DoradoFileSystemRoutingModule {}
