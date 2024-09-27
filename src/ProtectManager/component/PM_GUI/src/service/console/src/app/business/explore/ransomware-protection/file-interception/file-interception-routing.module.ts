import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { FileInterceptionComponent } from './file-interception.component';

const routes: Routes = [{ path: '', component: FileInterceptionComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class FileInterceptionRoutingModule {}
