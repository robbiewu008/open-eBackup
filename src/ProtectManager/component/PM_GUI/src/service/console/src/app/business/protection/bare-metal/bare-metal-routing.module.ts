import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { BareMetalComponent } from './bare-metal.component';

const routes: Routes = [
  {
    path: '',
    component: BareMetalComponent,
    children: [
      { path: '', redirectTo: 'fileset-template', pathMatch: 'full' },
      {
        path: 'fileset-template',
        loadChildren: () =>
          import(
            '../host-app/fileset/fileset-template-list/fileset-template-list.module'
          ).then(mod => mod.FilesetTemplateListModule),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class BareMetalRoutingModule {}
