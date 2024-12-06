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
  Input,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import {
  DefaultRoles,
  I18NService,
  MODAL_COMMON,
  ResourceSetApiService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, cloneDeep, find, isEmpty } from 'lodash';
import { CreateResourcesetComponent } from '../../../resource-set/create-resourceset/create-resourceset.component';

@Component({
  selector: 'aui-apply-resource',
  templateUrl: './apply-resource.component.html',
  styleUrls: ['./apply-resource.component.less']
})
export class ApplyResourceComponent implements OnInit, AfterViewInit {
  @Input() roleList;
  @Input() resourceSetMap;
  @Input() formGroup;
  selected;
  defaultRoleId;
  tableConfig;
  tableData = {
    data: [],
    total: 0
  };
  optsConfig: ProButton[];
  resourceSetTip;
  isNotLoginRole = false;

  @Output() invalidEmitter = new EventEmitter();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    private resourceSetService: ResourceSetApiService
  ) {}

  ngOnInit() {
    this.defaultRoleId = this.formGroup.get('roleId').value;
    this.initConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
    this.selected = this.roleList[0].roleId;
    this.resetSelection();
    this.invalidEmit();
  }

  initConfig() {
    this.optsConfig = [
      {
        id: 'create',
        label: this.i18n.get('system_create_resource_set_label'),
        type: 'primary',
        onClick: () => this.create()
      }
    ];
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'userNum',
        name: this.i18n.get('system_associated_users_num_label')
      },
      {
        key: 'description',
        name: this.i18n.get('common_desc_label')
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'uuid',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        fetchData: (filters: Filters) => {
          this.getData(filters);
        },
        selectionChange: selection => {
          this.resourceSetMap.set(this.selected, selection);
          this.invalidEmit();
        }
      }
    };
  }

  getData(filters) {
    const params = {
      pageNo: filters?.paginator.pageIndex,
      pageSize: filters?.paginator.pageSize
    };
    const conditionsTemp = {
      isPublic: false,
      isDefault: false
    };
    if (!isEmpty(filters.conditions_v2)) {
      assign(conditionsTemp, { ...JSON.parse(filters.conditions_v2) });
    }
    assign(params, { conditions: JSON.stringify(conditionsTemp) });
    this.resourceSetService.queryResourceSet(params).subscribe(res => {
      if (this.isNotLoginRole) {
        res.records.forEach(item =>
          assign(item, {
            disabled: true
          })
        );
      }
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
    });
  }

  create() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'create-resourceSet',
        lvWidth: MODAL_COMMON.xLargeWidth + 100,
        lvHeader: this.i18n.get('system_create_resource_set_label'),
        lvContent: CreateResourcesetComponent,
        lvOkDisabled: true,
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as CreateResourcesetComponent;
            const formValue = content.formGroup.value;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                res = this.selectNewResourceSet(res, formValue);
                this.dataTable.fetchData();
              },
              error: () => {
                resolve(false);
              }
            });
          });
        }
      })
    );
  }

  private selectNewResourceSet(res: any, formValue: any) {
    res = JSON.parse(res || '{}');
    const tmpNewResourceSet = {
      ...res,
      name: formValue.name,
      description: formValue.desc
    };
    // 新创出来的资源集默认选中
    if (this.resourceSetMap.has(this.selected)) {
      this.resourceSetMap.set(this.selected, [
        ...this.resourceSetMap.get(this.selected),
        tmpNewResourceSet
      ]);
    } else {
      this.resourceSetMap.set(this.selected, tmpNewResourceSet);
    }
    this.dataTable.setSelections(
      cloneDeep(this.resourceSetMap.get(this.selected))
    );
    this.invalidEmit();
    return res;
  }

  resetSelection() {
    this.isNotLoginRole = [
      DefaultRoles.rdAdmin.roleId,
      DefaultRoles.drAdmin.roleId,
      DefaultRoles.audit.roleId,
      DefaultRoles.sysAdmin.roleId
    ].includes(this.selected);
    this.tableData.data.forEach(item => (item.disabled = this.isNotLoginRole));
    this.dataTable.setSelections(this.resourceSetMap.get(this.selected));
    const roleName = find(this.roleList, { roleId: this.selected }).roleName;
    this.resourceSetTip = this.i18n.get(
      this.isNotLoginRole
        ? 'system_not_auth_resource_set_tip_label'
        : 'system_auth_resource_set_tip_label',
      [roleName]
    );
  }

  /**
   * 用于发射禁用/启用“下一步”按钮信号
   */
  invalidEmit() {
    this.invalidEmitter.emit(
      !this.roleList.every(
        item =>
          item.roleId === this.defaultRoleId ||
          [
            DefaultRoles.rdAdmin.roleId,
            DefaultRoles.drAdmin.roleId,
            DefaultRoles.audit.roleId,
            DefaultRoles.sysAdmin.roleId
          ].includes(item.roleId) ||
          this.resourceSetMap.get(item.roleId).length
      )
    );
  }
}
