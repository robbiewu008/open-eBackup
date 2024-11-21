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
import { AlertModule, ModalService } from '@iux/live';
import { BaseModule } from 'app/shared';
import { CurrentSystemTimeModule } from 'app/shared/components/current-system-time/current-system-time.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import {
  ProFilterSearchModule,
  ProTableModule
} from 'app/shared/components/pro-table';
import { AddBackupNodeModule } from './add-backup-node/add-backup-node.module';
import { AddHaModule } from './add-ha/add-ha.module';
import { AddNetworkModule } from './add-network/add-network.module';
import { AddTargetClusterModule } from './add-target-cluster/add-target-cluster.module';
import { AsManagementClusterModule } from './as-management-cluster/as-management-cluster.module';
import { AuthUserModule } from './auth-user/auth-user.module';
import { BackupClusterDistributedComponent } from './backup-cluster-distributed/backup-cluster-distributed.component';
import { BackupClusterComponent } from './backup-cluster/backup-cluster.component';
import { BackupNodeDetailDistributedComponent } from './backup-node-detail-distributed/backup-node-detail-distributed.component';
import { BackupNodeDetailComponent } from './backup-node-detail/backup-node-detail.component';
import { BackupNodeEditDistributedComponent } from './backup-node-edit-distributed/backup-node-edit-distributed.component';
import { CancleAuthUserModule } from './cancle-auth-user/cancle-auth-user.module';
import { ClusterDetailDistributedComponent } from './cluster-detail-distributed/cluster-detail-distributed.component';
import { ClusterDetailModule } from './cluster-detail/cluster-detail.module';
import { ClusterManagementRoutingModule } from './cluster-management-routing.module';
import {
  ClusterManagementComponent,
  ModifyClusterModalComponent,
  SelectionPipe
} from './cluster-management.component';
import { ConfigureDataplaneIpModule } from './configure-dataplane-ip/configure-dataplane-ip.module';
import { DeleteHaModule } from './delete-ha/delete-ha.module';
import { SelectQosPolicyModule } from './select-qos-policy/select-qos-policy.module';
import { TargetClusterComponent } from './target-cluster/target-cluster.component';
import { NodeNetworkEditDistributedComponent } from './node-network-edit-distributed/node-network-edit-distributed.component';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [
    ClusterManagementComponent,
    SelectionPipe,
    ModifyClusterModalComponent,
    TargetClusterComponent,
    BackupClusterComponent,
    BackupNodeDetailComponent,
    BackupNodeDetailDistributedComponent,
    BackupNodeEditDistributedComponent,
    NodeNetworkEditDistributedComponent,
    ClusterDetailDistributedComponent,
    BackupClusterDistributedComponent
  ],

  imports: [
    CommonModule,
    BaseModule,
    ClusterManagementRoutingModule,
    AddTargetClusterModule,
    ConfigureDataplaneIpModule,
    SelectQosPolicyModule,
    ClusterDetailModule,
    AsManagementClusterModule,
    MultiClusterSwitchModule,
    AuthUserModule,
    CancleAuthUserModule,
    AddHaModule,
    DeleteHaModule,
    CurrentSystemTimeModule,
    AlertModule,
    AddBackupNodeModule,
    ProTableModule,
    AddNetworkModule,
    ProFilterSearchModule,
    ProButtonModule,
    CustomTableSearchModule
  ],
  providers: [ModalService]
})
export class ClusterManagementModule {}
