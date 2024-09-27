import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { CopyApplicationComponent } from 'app/business/explore/copy-data/copy-application/copy-application.component';

const routes: Routes = [
  {
    path: '',
    component: CopyApplicationComponent,
    children: [
      {
        path: 'active-directory',
        loadChildren: () =>
          import('../active-directory/active-directory.module').then(
            mod => mod.ActiveDirectoryModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'exchange',
        loadChildren: () =>
          import('../exchange/exchange.module').then(mod => mod.ExchangeModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'sap-hana',
        loadChildren: () =>
          import('../saphana/saphana.module').then(mod => mod.SaphanaModule),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class CopyApplicationRoutingModule {}
