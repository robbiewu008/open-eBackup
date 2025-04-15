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
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormGroup } from '@angular/forms';
import {
  DataMap,
  DataMapService,
  I18NService,
  RoleApiService,
  SpecialRoleIds
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { assign, cloneDeep, defer, each, find, isEmpty, map } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-role',
  templateUrl: './add-role.component.html',
  styleUrls: ['./add-role.component.less']
})
export class AddRoleComponent implements OnInit, AfterViewInit {
  data;
  userType;
  type = 'old';
  selectionData;
  roleTableData;
  disabledData;
  roleTableConfig: TableConfig;
  formGroup = new FormGroup({});
  modalInValid = new EventEmitter();

  @ViewChild('roleDataTable', { static: false })
  roleDataTable: ProTableComponent;
  @ViewChild('treeTableTpl', { static: true }) treeTableTpl: TemplateRef<any>;
  @ViewChild('roleNameTpl', { static: true }) roleNameTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    public roleApiService: RoleApiService,
    public dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.disabledData = find(this.selectionData, { disabled: true });
    this.initConfig();
  }

  ngAfterViewInit() {
    this.roleDataTable.fetchData();
    this.roleDataTable.setSelections(cloneDeep(this.selectionData));
    this.formGroup.statusChanges.subscribe(status => {
      this.modalInValid.emit(status === 'INVALID' && this.type === 'new');
    });
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'roleName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.roleNameTpl
      },
      {
        key: 'userNum',
        name: this.i18n.get('system_associated_users_num_label'),
        sort: true
      },
      {
        key: 'roleDescription',
        name: this.i18n.get('common_desc_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];

    this.roleTableConfig = {
      table: {
        compareWith: 'roleId',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true,
          expandable: true,
          expandContent: this.treeTableTpl
        },
        selectionChange: selection => {
          if (Math.abs(selection.length - this.selectionData.length) > 1) {
            // 假如是全选，我们需要把那四个内置的直接置灰并且不选择
            each(this.roleTableData.data, val => {
              assign(val, {
                disabled:
                  val.roleId === this.data[0]?.roleId ||
                  SpecialRoleIds.includes(val.roleId)
              });
            });
            this.selectionData = selection.filter(
              item => !SpecialRoleIds.includes(item.roleId)
            );
            this.roleDataTable.setSelections(cloneDeep(this.selectionData));
          } else {
            this.selectionData = selection;
            // 除数据保护管理员外的其他内置角色只能单一选择
            this.parseDefaultRole();
          }
        },
        fetchData: (filter: Filters) => {
          this.getData(filter);
        }
      }
    };
  }

  private parseDefaultRole() {
    // 修改场景下如果原本的默认角色不是四个内置角色之一，就直接把那四个置灰，因为不能修改默认角色
    if (!SpecialRoleIds.includes(this.data[0]?.roleId)) {
      each(this.roleTableData.data, val => {
        assign(val, {
          disabled: val?.disabled || SpecialRoleIds.includes(val.roleId)
        });
      });
      return;
    }
    // 修改场景下如果选的那四个内置角色之一就全部置灰
    each(this.roleTableData.data, item => {
      assign(item, {
        disabled: true
      });
    });
  }

  typeChange(event) {
    this.modalInValid.emit(
      event === 'new' && this.formGroup.status === 'INVALID'
    );
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
      if (this.disabledData) {
        const disabledData = find(res.records, {
          roleId: this.disabledData.roleId
        });
        if (disabledData) {
          disabledData['disabled'] = true;
        }
      }
      this.roleTableData = {
        data: this.dataProcess(res),
        total: res.totalCount
      };

      defer(() => this.parseDefaultRole());
    });
  }

  private dataProcess(res) {
    return map(res.records, item => {
      item.originRoleName = item.roleName;
      if (
        this.userType !== DataMap.loginUserType.local.value &&
        [
          DataMap.defaultRoleName.rdAdmin.value,
          DataMap.defaultRoleName.drAdmin.value
        ].includes(item.roleName)
      ) {
        // 非本地用户则不选远端和灾备管理员
        item.disabled = true;
      }
      // 默认用户在修改的时候不能干掉
      if (item.roleId === this.data[0].roleId) {
        item.disabled = true;
      }
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

      return item;
    });
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.type === 'old') {
        observer.next(this.selectionData);
        observer.complete();
      } else {
        this.newRoleConfig(observer);
      }
    });
  }

  private newRoleConfig(observer: any) {
    const request = this.formGroup.getRawValue();
    this.roleApiService
      .createRole({
        request: request
      })
      .subscribe({
        next: res => {
          this.selectionData.push({
            roleId: JSON.parse(res)?.uuid,
            roleName: this.formGroup.get('roleName').value,
            roleDescription: this.formGroup.get('roleDescription').value,
            userNum: 0
          });
          observer.next(this.selectionData);
          observer.complete();
        },
        error: err => {
          observer.error(err);
          observer.complete();
        }
      });
  }
}
