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
import { AlertModule, CardModule, LinkModule } from '@iux/live';
import { BaseModule } from 'app/shared/base.module';
import { CustomModalOperateModule } from 'app/shared/components';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { BackupStorageUnitModule } from '../../infrastructure/backup-storage/backup-storage-unit/backup-storage-unit.module';
import { DistributedNasListModule } from '../../infrastructure/backup-storage/distributed-nas-list.module';
import { RbacOverviewTagComponent } from './rbac-overview-tag/rbac-overview-tag.component';
import { RbacRoutingModule } from './rbac-routing.module';
import { RbacComponent } from './rbac.component';
import { ResourceSetDetailModule } from './resource-set/resource-set-detail/resource-set-detail.module';
import { ResourceSetModule } from './resource-set/resource-set.module';
import { CreateRoleFormComponent } from './roles/create-role/create-role-form/create-role-form.component';
import { CreateRoleComponent } from './roles/create-role/create-role.component';
import { AssociatedUsersComponent } from './roles/role-detail/associated-users/associated-users.component';
import { RoleAuthTreeComponent } from './roles/role-detail/role-auth-tree/role-auth-tree.component';
import { RoleDetailComponent } from './roles/role-detail/role-detail.component';
import { RolesComponent } from './roles/roles.component';
import { AddRoleComponent } from './users/create-user/add-role/add-role.component';
import { ApplyResourceComponent } from './users/create-user/apply-resource/apply-resource.component';
import { CreateUserComponent } from './users/create-user/create-user.component';
import { ResetpwdComponent } from './users/resetpwd/resetpwd.component';
import { UserDetailFormComponent } from './users/user-detail/user-detail-form/user-detail-form.component';
import { UserDetailComponent } from './users/user-detail/user-detail.component';
import { UsersComponent } from './users/users.component';
@NgModule({
  declarations: [
    RbacComponent,
    RolesComponent,
    CreateRoleComponent,
    RoleDetailComponent,
    AssociatedUsersComponent,
    UsersComponent,
    CreateUserComponent,
    UserDetailComponent,
    AddRoleComponent,
    CreateRoleFormComponent,
    UserDetailFormComponent,
    ApplyResourceComponent,
    ResetpwdComponent,
    RoleAuthTreeComponent,
    RbacOverviewTagComponent
  ],
  imports: [
    CommonModule,
    RbacRoutingModule,
    BaseModule,
    CustomModalOperateModule,
    MultiClusterSwitchModule,
    ProButtonModule,
    ProTableModule,
    ResourceSetModule,
    ResourceSetDetailModule,
    CardModule,
    AlertModule,
    LinkModule,
    DistributedNasListModule,
    BackupStorageUnitModule
  ]
})
export class RbacModule {}
