import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetectionSettingRoutingModule } from './detection-setting-routing.module';
import { DetectionSettingComponent } from './detection-setting.component';
import { BaseModule } from 'app/shared';
import { DetectionModelListModule } from '../detection-model-list/detection-model-list.module';
import { DetectionSettingListModule } from '../detection-setting-list/detection-setting-list.module';
import { BlockingRuleListModule } from '../blocking-rule-list/blocking-rule-list.module';
import { DetectionWhitelistModule } from '../detection-whitelist/detection-whitelist.module';
import { SanDetectionSettingListModule } from '../san-detection-setting-list/san-detection-setting-list.module';

@NgModule({
  declarations: [DetectionSettingComponent],
  imports: [
    CommonModule,
    BaseModule,
    DetectionSettingListModule,
    DetectionSettingRoutingModule,
    DetectionWhitelistModule,
    SanDetectionSettingListModule
  ]
})
export class DetectionSettingModule {}
