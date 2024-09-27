import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ElasticSearchComponent } from './elastic-search.component';

const routes: Routes = [{ path: '', component: ElasticSearchComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class HiveRoutingModule {}
