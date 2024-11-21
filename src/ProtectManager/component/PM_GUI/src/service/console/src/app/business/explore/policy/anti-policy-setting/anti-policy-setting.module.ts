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
