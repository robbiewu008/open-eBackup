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
import { BackupSetComponent } from './backup-set.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { CreateBackupSetModule } from './create-backup-set/create-backup-set.module';
import { SummaryModule } from './summary/summary.module';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { JobResourceModule } from 'app/business/insight/job/job-resource/job-resource.module';
import { CopyDataModule } from './copy-data/copy-data.module';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { CreateBackupsetModule } from '../../hive/create-backupset/create-backupset.module';

@NgModule({
  declarations: [BackupSetComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    CreateBackupSetModule,
    ProtectModule,
    CopyDataModule,
    JobResourceModule,
    DetailModalModule,
    SlaServiceModule,
    SummaryModule,
    CreateBackupsetModule
  ],
  exports: [BackupSetComponent]
})
export class BackupSetModule {}
