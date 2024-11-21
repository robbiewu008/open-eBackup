/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
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
