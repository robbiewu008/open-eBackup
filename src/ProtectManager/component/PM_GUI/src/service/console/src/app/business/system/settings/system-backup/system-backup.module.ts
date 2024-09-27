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
import { BaseModule } from 'app/shared/base.module';
import { PolicyConfigModule } from './policy-config/policy-config.module';
import { SystemBackupRoutingModule } from './system-backup-routing.module';
import { SystemBackupComponent } from './system-backup.component';
import { ManuallBackupModule } from './manuall-backup/manuall-backup.module';
import { BackupRestoreModule } from './backup-restore/backup-restore.module';
import { ImportBackupModule } from './import-backup/import-backup.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [SystemBackupComponent],
  imports: [
    BaseModule,
    SystemBackupRoutingModule,
    ManuallBackupModule,
    PolicyConfigModule,
    BackupRestoreModule,
    ImportBackupModule,
    MultiClusterSwitchModule
  ]
})
export class SystemBackupModule {}
