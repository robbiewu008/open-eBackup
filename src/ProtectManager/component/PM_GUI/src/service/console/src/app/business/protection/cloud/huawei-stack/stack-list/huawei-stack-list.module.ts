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
import { BaseModule } from 'app/shared';
import { HuaWeiStackListComponent } from './huawei-stack-list.component';
import { TakeManualBackupServiceModule } from 'app/shared/services/take-manual-backup.service';
import { WarningBatchConfirmModule } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.module';
import { AddTelnetComputeModule } from './add-telnet/add-telnet.module';
import { SelectDatabaseListModule } from './select-database-list/select-database-list.module';
import { CloudStackAdvancedParameterModule } from './stack-advanced-parameter/cloud-stack-advanced-parameter.module';
import { HCSHostSummaryModule } from './host-summary/host-summary.module';
import { ProjectSummaryModule } from './project-summary/project-summary.module';
import { TenantDetailModule } from './tenant-detail/tenant-detail.module';
import { HCSCopyDataModule } from './copy-data/hcs-copy-data.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [HuaWeiStackListComponent],
  imports: [
    BaseModule,
    TakeManualBackupServiceModule,
    WarningBatchConfirmModule,
    AddTelnetComputeModule,
    SelectDatabaseListModule,
    CloudStackAdvancedParameterModule,
    HCSHostSummaryModule,
    HCSCopyDataModule,
    ProjectSummaryModule,
    TenantDetailModule,
    CustomTableSearchModule
  ],
  exports: [HuaWeiStackListComponent]
})
export class HuaWeiStackListModule {}
