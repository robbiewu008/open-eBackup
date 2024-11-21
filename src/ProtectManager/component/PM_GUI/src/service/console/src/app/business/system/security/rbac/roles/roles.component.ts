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
  EventEmitter,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CookieService,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  OperateItems,
  RoleApiService,
  RoleType,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, isEmpty, map } from 'lodash';

@Component({
  selector: 'aui-roles',
  templateUrl: './roles.component.html',
  styleUrls: ['./roles.component.less']
})
export class RolesComponent implements OnInit, AfterViewInit {
  optsConfig;
  tableConfig;
  tableData: TableData;
  selectionData = [];
  isSysAdmin = this.cookieService.role === RoleType.SysAdmin;

  @Output() openPage = new EventEmitter();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('roleNameTpl', { static: true }) roleNameTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    public roleApiService: RoleApiService,
    public virtualScroll: VirtualScrollService,
    public warningMessageService: WarningMessageService,
    private cookieService: CookieService,
    public dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'create',
        type: 'primary',
        permission: OperateItems.SysadminOnly,
        label: this.i18n.get('common_create_label'),
        onClick: () => {
          this.create();
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.SysadminOnly,
        onClick: data => {
          this.delete(data);
        },
        disableCheck: data => !data.length
      }
    ];
    this.optsConfig = getPermissionMenuItem(opts, this.cookieService.role);
    const itemOpts: ProButton[] = [
      {
        id: 'modify',
        label: this.i18n.get('common_modify_label'),
        permission: OperateItems.SysadminOnly,
        onClick: rowData => {
          this.create(rowData[0], true, false);
        },
        disableCheck: rowData => rowData[0].is_default
      },
      {
        id: 'clone',
        label: this.i18n.get('common_clone_label'),
        permission: OperateItems.SysadminOnly,
        onClick: rowData => {
          this.create(rowData[0], false, true);
        }
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.SysadminOnly,
        onClick: rowData => {
          this.delete(rowData);
        },
        disableCheck: rowData => !!rowData[0].userNum || rowData[0].is_default
      }
    ];
    const cols: TableCols[] = [
      {
        key: 'roleName',
        name: this.i18n.get('common_name_label'),
        width: '300px',
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.roleNameTpl
      },
      {
        key: 'userNum',
        name: this.i18n.get('system_associated_users_num_label'),
        width: '200px',
        sort: true
      },
      {
        key: 'roleDescription',
        name: this.i18n.get('common_desc_label')
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        hidden: !this.isSysAdmin,
        width: '200px',
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 2,
            items: getPermissionMenuItem(itemOpts, this.cookieService.role)
          }
        }
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'roleId',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: this.isSysAdmin
        },
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        }
      }
    };
  }

  create(rowData?, isModify?, isClone?) {
    this.openPage.emit({
      name: 'createRole',
      data: {
        rowData: rowData,
        isModify: isModify ?? false,
        isClone: isClone ?? false
      }
    });
  }

  delete(datas: any[]) {
    this.warningMessageService.create({
      content: this.i18n.get('system_delete_role_tip_label', [
        datas.map(item => item.roleName).join(',')
      ]),
      onOK: () => {
        this.roleApiService
          .deleteRole({
            RoleDeleteRequest: {
              roleIdList: map(datas, 'roleId')
            }
          })
          .subscribe(() => {
            this.dataTable.fetchData();
            this.selectionData = [];
            this.dataTable.setSelections([]);
          });
      }
    });
  }

  detail(data) {
    this.openPage.emit({
      name: 'roleDetail',
      data: data
    });
  }

  getData(filter) {
    const params = {
      pageNo: filter.paginator.pageIndex,
      pageSize: filter.paginator.pageSize
    };
    const conditions = !isEmpty(filter.conditions)
      ? JSON.parse(filter.conditions)
      : {};
    if (conditions) {
      assign(params, {
        conditions: JSON.stringify({
          roleName: conditions.roleName,
          language: this.i18n.language === 'zh-cn' ? 'cn' : 'en'
        })
      });
    }
    if (filter.sort?.direction) {
      const direction = filter.sort.direction === 'asc' ? '+' : '-';
      assign(params, {
        orders: [direction + 'user_num']
      });
    }
    this.roleApiService.getUsingGET(params).subscribe(res => {
      this.tableData = {
        data: this.dataProcess(res),
        total: res.totalCount
      };
    });
  }

  private dataProcess(res) {
    return map(res.records, item => {
      if (item['is_default']) {
        item.roleName = this.dataMapService.getLabel(
          'defaultRoleName',
          item.roleName
        );
        item['roleDescription'] = this.dataMapService.getLabel(
          'defaultRoleDescription',
          item['roleDescription']
        );
      } else {
        // 升级场景会有自动创造的角色，使用词条做描述
        item.roleDescription = this.i18n.get(item.roleDescription);
      }
      return assign(item, { disabled: !!item.userNum || item['is_default'] });
    });
  }
}
