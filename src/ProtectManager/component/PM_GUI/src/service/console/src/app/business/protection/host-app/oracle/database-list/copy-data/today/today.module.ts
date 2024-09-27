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
import { OracleRestoreModule } from 'app/business/protection/host-app/oracle/database-list/copy-data/today/oracle-restore/oracle-restore.module';
import { BaseModule } from 'app/shared/base.module';
import { CopyDataListModule } from 'app/shared/components/copy-data-list/copy-data-list.module';
import { ManualMountModule } from 'app/shared/services/manual-mount.service';
import { SummaryModule } from './summary/summary.module';
import { TodayComponent } from './today.component';

@NgModule({
  declarations: [TodayComponent],
  imports: [
    BaseModule,
    OracleRestoreModule,
    SummaryModule,
    CopyDataListModule,
    ManualMountModule
  ],
  exports: [TodayComponent]
})
export class TodayModule {}
