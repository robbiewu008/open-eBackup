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
import { EdituserComponent } from './edituser/edituser.component';
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { UserroleRoutingModule } from './user-role-routing.module';
import { UserroleComponent } from './user-role.component';
import { AssociatedusersComponent } from './associatedusers/associatedusers.component';
import { CreateuserComponent } from './createuser/createuser.component';
import { ResetpwdComponent } from './resetpwd/resetpwd.component';
import { UserdetailComponent } from './userdetail/userdetail.component';
import { UnlockComponent } from './unlock/unlock.component';
import { CustomModalOperateModule } from 'app/shared/components';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { SetEmailComponent } from './set-email/set-email.component';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';
@NgModule({
  declarations: [
    UserroleComponent,
    AssociatedusersComponent,
    CreateuserComponent,
    EdituserComponent,
    ResetpwdComponent,
    UserdetailComponent,
    SetEmailComponent,
    UnlockComponent
  ],
  imports: [
    CommonModule,
    UserroleRoutingModule,
    BaseModule,
    CustomModalOperateModule,
    MultiClusterSwitchModule,
    CustomTableSearchModule
  ]
})
export class UserroleModule {}
