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
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { TakeManualBackupServiceModule } from 'app/shared/services/take-manual-backup.service';
import { CopyDataModule } from './copy-data/copy-data.module';
import { CreateFilesetModule } from './create-fileset/create-fileset.module';
import { FilesetRoutingModule } from './fileset-routing.module';
import { FilesetComponent } from './fileset.component';
import { SummaryModule } from './summary/summary.module';
import { ResourceDetailService } from 'app/shared/services/resource-detail.service';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { WarningBatchConfirmModule } from 'app/shared/components/warning-batch-confirm/warning-batch-confirm.module';
import { CreateFilesetTemplateModule } from './fileset-template-list/create-fileset-template/create-fileset-template.module';
import { FilesetRestoreModule } from './fileset-restore/fileset-restore.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';
@NgModule({
  declarations: [FilesetComponent],
  imports: [
    CommonModule,
    SummaryModule,
    BaseModule,
    CopyDataModule,
    DetailModalModule,
    SlaServiceModule,
    FilesetRoutingModule,
    CreateFilesetModule,
    ProtectModule,
    FilesetRestoreModule,
    CreateFilesetTemplateModule,
    TakeManualBackupServiceModule,
    WarningBatchConfirmModule,
    CustomTableSearchModule
  ],
  providers: [ResourceDetailService],
  exports: [FilesetComponent]
})
export class FilesetModule {}
