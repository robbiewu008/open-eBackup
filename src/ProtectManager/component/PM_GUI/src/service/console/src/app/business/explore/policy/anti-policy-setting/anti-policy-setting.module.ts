import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DetectionModelListModule } from '../../anti-ransomware/detection-model-list/detection-model-list.module';
import { AntiPolicyModule } from '../anti-policy-setting/anti-policy/anti-policy.module';
import { AntiPolicySettingRoutingModule } from './anti-policy-setting-routing.module';
import { AntiPolicySettingComponent } from './anti-policy-setting.component';
import { InfectedCopyLimitModule } from './infected-copy-limit/infected-copy-limit.module';

@NgModule({
  declarations: [AntiPolicySettingComponent],
  imports: [
    CommonModule,
    BaseModule,
    AntiPolicySettingRoutingModule,
    AntiPolicyModule,
    DetectionModelListModule,
    InfectedCopyLimitModule
  ],
  exports: [AntiPolicySettingComponent]
})
export class AntiPolicySettingModule {}
