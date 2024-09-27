import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { AntiPolicySettingComponent } from './anti-policy-setting.component';

const routes: Routes = [{ path: '', component: AntiPolicySettingComponent }];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class AntiPolicySettingRoutingModule {}
