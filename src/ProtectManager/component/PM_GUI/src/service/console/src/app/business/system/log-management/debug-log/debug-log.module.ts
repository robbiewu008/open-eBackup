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
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { DebugLogRoutingModule } from './debug-log-routing.module';
import { DebugLogComponent } from './debug-log.component';
import { ExportFilesModule } from 'app/shared/components/export-files/export-files.module';
import { MultiLogModule } from '../multi-log/multi-log.module';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [DebugLogComponent],
  imports: [
    CommonModule,
    DebugLogRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    ExportFilesModule,
    MultiLogModule,
    AlertModule
  ]
})
export class DebugLogModule {}
