import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { CopyBareMetalComponent } from './copy-bare-metal.component';

const routes: Routes = [
  {
    path: '',
    component: CopyBareMetalComponent,
    children: [
      {
        path: '',
        redirectTo: 'fileset',
        pathMatch: 'full'
      },
      {
        path: 'fileset',
        loadChildren: () =>
          import('../fileset/fileset.module').then(mod => mod.FilesetModule),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CopyBareMetalRoutingModule {}
