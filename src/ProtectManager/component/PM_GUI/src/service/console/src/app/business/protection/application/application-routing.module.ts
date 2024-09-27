import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ApplicationComponent } from './application.component';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';

const routes: Routes = [
  {
    path: '',
    component: ApplicationComponent,
    children: [
      { path: '', redirectTo: 'active-directory', pathMatch: 'full' },
      {
        path: 'active-directory',
        loadChildren: () =>
          import('./active-directory/active-directory.module').then(
            mod => mod.ActiveDirectoryModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'exchange',
        loadChildren: () =>
          import('../host-app/exchange/exchange.module').then(
            mod => mod.ExchangeModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'sap-hana',
        loadChildren: () =>
          import('./saphana/saphana.module').then(mod => mod.SaphanaModule),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ApplicationRoutingModule {}
