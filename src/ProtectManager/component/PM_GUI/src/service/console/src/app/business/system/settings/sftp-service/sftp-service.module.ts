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
import { ProcessLoadingModule } from 'app/shared/components/process-loading/process-loading.module';
import { AddUserModule } from './add-user/add-user.module';
import { ChangePasswordModule } from './change-password/change-password.module';
import { SftpServiceRoutingModule } from './sftp-service-routing.module';
import { SftpServiceComponent } from './sftp-service.component';
import { StartSftpModule } from './start-sftp/start-sftp.module';
import { ThresholdModifyModule } from './threshold-modify/threshold-modify.module';
import { UserDetailModule } from './user-detail/user-detail.module';

@NgModule({
  declarations: [SftpServiceComponent],
  imports: [
    CommonModule,
    AddUserModule,
    UserDetailModule,
    ChangePasswordModule,
    ProcessLoadingModule,
    SftpServiceRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    ThresholdModifyModule,
    StartSftpModule
  ]
})
export class SftpServiceModule {}
