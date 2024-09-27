import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { RedisShowComponent } from './redis-show.component';

const routes: Routes = [{ path: '', component: RedisShowComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class RedisShowRoutingModule {}
