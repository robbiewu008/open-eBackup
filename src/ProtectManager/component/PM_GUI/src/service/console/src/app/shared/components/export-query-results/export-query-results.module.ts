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
import { MultiClusterSwitchModule } from '../multi-cluster-switch/multi-cluster-switch.module';
import { ProTableModule } from '../pro-table';
import { ExportQueryResultsComponent } from './export-query-results.component';

@NgModule({
  declarations: [ExportQueryResultsComponent],
  imports: [CommonModule, BaseModule, MultiClusterSwitchModule, ProTableModule],

  exports: [ExportQueryResultsComponent]
})
export class ExportQueryResultsModule {}
