import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { InsightComponent } from './insight.component';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';

const routes: Routes = [
  {
    path: '',
    component: InsightComponent,
    children: [
      { path: '', redirectTo: 'insight/performance', pathMatch: 'full' },
      {
        path: 'performance',
        loadChildren: () =>
          import('./performance/performance.module').then(
            mod => mod.PerformanceModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'alarms',
        loadChildren: () =>
          import('./alarms/alarms.module').then(mod => mod.AlarmsModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'jobs',
        loadChildren: () =>
          import('./job/job.module').then(mod => mod.JobModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'reports',
        loadChildren: () =>
          import('./reports/reports.module').then(mod => mod.ReportsModule),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class InsightRoutingModule {}
