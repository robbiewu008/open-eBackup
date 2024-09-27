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
import { Component, EventEmitter, OnInit } from '@angular/core';
import { CookieService, I18NService, RoleType } from 'app/shared';
import { ResourceSetDetailComponent } from './resource-set/resource-set-detail/resource-set-detail.component';
import { CreateRoleComponent } from './roles/create-role/create-role.component';
import { CreateUserComponent } from './users/create-user/create-user.component';
import { RoleDetailComponent } from './roles/role-detail/role-detail.component';
import { UserDetailComponent } from './users/user-detail/user-detail.component';

@Component({
  selector: 'aui-rbac',
  templateUrl: './rbac.component.html',
  styleUrls: ['./rbac.component.less']
})
export class RbacComponent implements OnInit {
  openPageEmitter = new EventEmitter();
  subComponentMap = {};
  subComponent = null;
  subComponentContext = {
    openPage: this.openPageEmitter,
    data: null
  };
  tabActiveIndex;
  isDPAdmin = ![RoleType.SysAdmin, RoleType.Auditor].includes(
    this.cookieService.role
  );

  constructor(public i18n: I18NService, private cookieService: CookieService) {}

  ngOnInit() {
    this.openPageEmitter.subscribe(cfg => this.openPage(cfg));
    this.subComponentMap = {
      createRole: CreateRoleComponent,
      roleDetail: RoleDetailComponent,
      createUser: CreateUserComponent,
      userDetail: UserDetailComponent,
      resourceSetDetail: ResourceSetDetailComponent
    };
  }

  openPage(pageConfig?) {
    if (!pageConfig) {
      this.subComponent = null;
      this.subComponentContext.data = null;
      return;
    }
    this.subComponent = this.subComponentMap[pageConfig.name];
    this.subComponentContext.data = pageConfig.data;
  }
}
