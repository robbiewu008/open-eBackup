import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { BlockingRuleListComponent } from './blocking-rule-list.component';

const routes: Routes = [{ path: '', component: BlockingRuleListComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class BlocingRuleListRoutingModule {}
