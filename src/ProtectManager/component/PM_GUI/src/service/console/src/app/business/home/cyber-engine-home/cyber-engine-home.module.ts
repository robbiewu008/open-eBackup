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
import { CyberEngineHomeComponent } from './cyber-engine-home.component';
import { BigScreenModule } from './big-screen/big-screen.module';
import { ExceptionsFileModule } from './exceptions-file/exceptions-file.module';
import { AlarmsCountModule } from './alarms-count/alarms-count.module';
import { FileInterceptionModule } from './file-interception/file-interception.module';
import { TaskCountModule } from './task-count/task-count.module';
import { AirGapModule } from './air-gap/air-gap.module';
import { LicenseCapacityModule } from './license-capacity/license-capacity.module';
import { BaseModule } from 'app/shared';

@NgModule({
  imports: [
    CommonModule,
    BaseModule,
    BigScreenModule,
    ExceptionsFileModule,
    AlarmsCountModule,
    FileInterceptionModule,
    LicenseCapacityModule,
    TaskCountModule,
    AirGapModule
  ],
  declarations: [CyberEngineHomeComponent],
  exports: [CyberEngineHomeComponent]
})
export class CyberEngineHomeModule {}
