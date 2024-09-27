import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { HomeComponent } from './home.component';
import { LayoutModule, GroupModule, BadgeModule } from '@iux/live';

const routes: Routes = [{ path: '', component: HomeComponent }];

@NgModule({
  imports: [
    RouterModule.forChild(routes),
    LayoutModule,
    GroupModule,
    BadgeModule
  ],
  exports: [RouterModule]
})
export class HomeRouterModule {}
