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
import { Component, Input, OnInit, ViewChild } from '@angular/core';
import {
  CommonConsts,
  UserRoleType,
  UserRoleI18nMap,
  UsersApiService
} from 'app/shared';
import { I18NService } from 'app/shared/services';
import { assign, isEmpty, each, filter, toString, trim, isNil } from 'lodash';
import { map } from 'rxjs/operators';
import { DatatableComponent } from '@iux/live';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-user-list',
  templateUrl: './user-list.component.html',
  styleUrls: ['./user-list.component.less']
})
export class UserListComponent implements OnInit {
  userName;
  tableData = [];
  filterParams = { roleName: ['Role_DP_Admin'] };
  sortSources = {};

  selection$ = new Subject<any>();
  userRoleType = UserRoleType;
  userRoleI18nMap = UserRoleI18nMap;

  startIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;

  columns = [
    {
      key: 'userName',
      label: this.i18n.get('common_name_label')
    },
    {
      key: 'roleName',
      label: this.i18n.get('common_role_label'),
      filter: false,
      filterMap: [
        {
          value: 'Role_SYS_Admin',
          label: this.i18n.get('common_sys_admin_label'),
          key: 'Role_SYS_Admin'
        },
        {
          value: 'Role_DP_Admin',
          label: this.i18n.get('common_user_label'),
          key: 'Role_DP_Admin'
        },
        {
          value: 'Role_Auditor',
          label: this.i18n.get('common_auditor_label'),
          key: 'Role_Auditor'
        },
        {
          value: 'Role_RD_Admin',
          label: this.i18n.get('common_remote_device_administrator_label'),
          key: 'Role_RD_Admin'
        },
        {
          value: 'Role_DR_Admin',
          label: this.i18n.get('common_dme_admin_label'),
          key: 'Role_DR_Admin'
        }
      ]
    },
    {
      key: 'login',
      label: this.i18n.get('common_status_label')
    },
    {
      key: 'lock',
      label: this.i18n.get('common_lock_status_label')
    }
  ];

  @Input() type;
  @ViewChild('lvTable', { static: false }) lvTable: DatatableComponent;

  constructor(
    private i18n: I18NService,
    private usersApiService: UsersApiService
  ) {}

  ngOnInit() {
    this.getUsers();
    this.initColumns();
  }
  private initColumns() {
    const deployType = this.i18n.get('deploy_type');
    if (['d3', 'd4', 'cloudbackup'].includes(deployType)) {
      let _filterMap = this.columns.find(item => item.key === 'roleName')
        ?.filterMap;
      if (
        !isNil(this.columns.find(item => item.key === 'roleName').filterMap)
      ) {
        this.columns.find(
          item => item.key === 'roleName'
        ).filterMap = _filterMap.filter(item => item.key !== 'Role_DP_Admin');
      }
    }
  }

  getUsers() {
    const params = {
      startIndex: this.startIndex + 1,
      pageSize: this.pageSize
    };

    each(this.filterParams, (value, key) => {
      if (isEmpty(value)) {
        delete this.filterParams[key];
      }
    });

    if (!isEmpty(this.filterParams)) {
      assign(params, {
        filter: JSON.stringify(this.filterParams)
      });
    }

    if (!isEmpty(this.sortSources)) {
      assign(params, {
        orderBy: this.sortSources['key'],
        orderType: this.sortSources['direction']
      });
    }

    this.usersApiService
      .getAllUserUsingGET(params)
      .pipe(
        map(res => {
          filter(res.userList, (user: any) => {
            user.login = toString(user.login);
            return true;
          });
          return res;
        })
      )
      .subscribe(res => {
        this.total = res.total;
        this.tableData = res.userList;
      });
  }

  sortChange(source) {
    this.sortSources = source;
    this.getUsers();
  }

  filterChange(e) {
    assign(this.filterParams, {
      [e.key]: e.value
    });
    this.getUsers();
  }

  selectionRow(source) {
    this.lvTable.toggleSelection(source);
    this.selection$.next(this.lvTable.getSelection());
  }

  searchByUserName(userName) {
    assign(this.filterParams, {
      userName: trim(userName)
    });

    this.getUsers();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.startIndex = page.pageIndex;
    this.getUsers();
  }
}
