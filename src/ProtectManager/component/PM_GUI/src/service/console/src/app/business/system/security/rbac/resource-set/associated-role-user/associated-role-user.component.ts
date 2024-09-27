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
import {
  AfterViewInit,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { DataMapService, I18NService, ResourceSetApiService } from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, find } from 'lodash';

@Component({
  selector: 'aui-associated-role-user',
  templateUrl: './associated-role-user.component.html',
  styleUrls: ['./associated-role-user.component.less']
})
export class AssociatedRoleUserComponent implements OnInit, AfterViewInit {
  @Input() data;

  tableData: TableData;
  tableConfig: TableConfig;
  roleData: TableData;
  roleConfig: TableConfig;

  @ViewChild('userTable', { static: false })
  userTable: ProTableComponent;
  @ViewChild('expandTpl', { static: true }) expandTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private resourceSetService: ResourceSetApiService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.getData();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'userName',
        name: this.i18n.get('common_users_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'userDesc',
        name: this.i18n.get('system_user_desc_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];
    this.tableConfig = {
      table: {
        columns: cols,
        rows: {
          expandable: true,
          expandContent: this.expandTpl
        },
        compareWith: 'userName',
        async: false
      }
    };
    const roleCols: TableCols[] = [
      {
        key: 'roleName',
        name: this.i18n.get('common_role_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'roleDesc',
        name: this.i18n.get('system_role_desc_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];
    this.roleConfig = {
      table: {
        columns: roleCols,
        compareWith: 'roleName',
        async: false,
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple'
      }
    };
  }

  getData() {
    this.resourceSetService
      .QueryRelatedUserRoleByResourceSetId({ resourceSetId: this.data.uuid })
      .subscribe(res => {
        const userArray = [];
        res.forEach(item => {
          const roleName = this.dataMapService.getLabel(
            'defaultRoleName',
            item.roleName
          );
          item.roleName = roleName === '--' ? item.roleName : roleName;
          const roleDesc = this.dataMapService.getLabel(
            'defaultRoleDescription',
            item.roleDesc
          );
          item.roleDesc = roleDesc === '--' ? item.roleDesc : roleDesc;
          let tmpData = find(userArray, { userName: item.userName });
          if (!tmpData) {
            // 如果原来没有就塞
            userArray.push(
              assign(item, {
                roleData: {
                  data: [
                    {
                      roleName: item.roleName,
                      roleDesc: item.roleDesc
                    }
                  ],
                  total: 1
                }
              })
            );
          } else {
            tmpData.roleData.data.push({
              roleName: item.roleName,
              roleDesc: item.roleDesc
            });
            tmpData.roleData.total++;
          }
        });
        this.tableData = {
          data: userArray,
          total: userArray.length
        };
      });
  }
}
