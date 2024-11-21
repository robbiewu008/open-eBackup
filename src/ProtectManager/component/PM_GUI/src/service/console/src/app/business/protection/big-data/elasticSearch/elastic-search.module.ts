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
import { HiveRoutingModule } from './elastic-search-routing.module';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ElasticSearchComponent } from './elastic-search.component';
import { ClustersModule } from '../hbase/clusters/clusters.module';
import { BackupSetModule } from '../hbase/backup-set/backup-set.module';
import { CreateBackupsetModule } from './create-backupset/create-backupset.module';
@NgModule({
  declarations: [ElasticSearchComponent],
  imports: [
    CommonModule,
    HiveRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    ClustersModule,
    BackupSetModule,
    CreateBackupsetModule
  ]
})
export class ElasticSearchModule {}
