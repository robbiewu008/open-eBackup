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
import { Component, Input, OnInit } from '@angular/core';
import { I18NService, PermissionTable, RoleAuthApiService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { cloneDeep } from 'lodash';

@Component({
  selector: 'aui-role-auth-tree',
  templateUrl: './role-auth-tree.component.html',
  styleUrls: ['./role-auth-tree.component.less']
})
export class RoleAuthTreeComponent implements OnInit {
  @Input() data;
  tableData = [];

  constructor(
    public i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private roleAuthApiService: RoleAuthApiService
  ) {}

  ngOnInit() {
    this.getData();
  }

  getData() {
    this.roleAuthApiService
      .getAuthListByRoleIdUsingGet({
        id: this.data.roleId
      })
      .subscribe(res => {
        const authSet = new Set(res.map(item => item.uuid));
        const permissionTable = cloneDeep(PermissionTable).filter(
          item =>
            !(
              this.appUtilsService.isDistributed &&
              item.value === 'DataSecurity'
            )
        );
        permissionTable.forEach(
          item =>
            (item.children = item.children.filter(subItem =>
              authSet.has(subItem.value)
            ))
        );
        this.tableData = permissionTable.filter(item => item.children.length);
      });
  }
}
