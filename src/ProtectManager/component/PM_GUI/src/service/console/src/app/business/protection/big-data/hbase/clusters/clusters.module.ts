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
import { ClustersComponent } from './clusters.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ClusterDetailModule } from './cluster-detail/cluster-detail.module';
import { ClusterBackupsetDetailModule } from './cluster-backupset-detail/cluster-backupset-detail.module';
import { RegisterClusterModule } from '../../hdfs/clusters/register-cluster/register-cluster.module';
import { RegisterClusterModule as HiveRegisterClusterModule } from '../../hive/register-cluster/register-cluster.module';
import { RegisterClusterModule as EsRegisterClusterComponent } from '../../elasticSearch/register-cluster/register-cluster.module';
@NgModule({
  declarations: [ClustersComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ClusterDetailModule,
    ClusterBackupsetDetailModule,
    RegisterClusterModule,
    HiveRegisterClusterModule,
    EsRegisterClusterComponent
  ],
  exports: [ClustersComponent]
})
export class ClustersModule {}
