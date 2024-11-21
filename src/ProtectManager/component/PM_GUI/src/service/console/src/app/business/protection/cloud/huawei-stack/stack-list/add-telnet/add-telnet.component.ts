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
import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { LvConfig, MessageService, ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  GlobalService,
  I18NService,
  MODAL_COMMON,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';

import {
  assign,
  cloneDeep,
  each,
  find,
  set,
  isEmpty,
  isEqual,
  isNumber,
  remove,
  size
} from 'lodash';
import { AddUserComponent } from './add-user/add-user.component';

@Component({
  selector: 'aui-add-telnet',
  templateUrl: './add-telnet.component.html',
  styleUrls: ['./add-telnet.component.less']
})
export class AddTelnetComponent implements OnInit {
  private readonly MAX_LENGTH = 32;
  item;
  ip;

  DataMap = DataMap;
  formGroup: FormGroup;
  tableConfig: TableConfig;
  optItems = [];
  treeSelection;
  telnetOptions = [];

  tenant;
  data;
  domainId;
  isTest = false;
  tableData = {
    data: [],
    total: 0
  };
  resourceType = ResourceType;
  title: string;
  managerLabel: string;
  showTips = false;

  nameErrorTip = assign(this.baseUtilService.nameErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.MAX_LENGTH
    ])
  });
  pwdErrorTip = assign(this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.MAX_LENGTH
    ])
  });

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    public dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private messageService: MessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private globalService: GlobalService
  ) {}

  ngOnInit() {
    this.globalService.getState('add-tenant-user').subscribe(res => {
      const findItem = find(this.tableData.data, { name: res.name });
      if (isEmpty(findItem)) {
        this.tableData = {
          data: [...this.tableData.data, res],
          total: size([...this.tableData.data, res])
        };
      } else {
        assign(findItem, { passwd: res.passwd });
        this.tableData = {
          data: this.tableData.data,
          total: size(this.tableData.data)
        };
      }
      this.formGroup.get('children').setValue(this.tableData.data);
    });
    this.initForm();
    this.updateData(); // 回显数据
    this.initConfig();
    this.getTenantOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      platform_telnet: new FormControl(
        isEmpty(this.item) ? '' : this.item.uuid,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      children: new FormControl([], {
        validators: [this.baseUtilService.VALID.minLength(1)]
      }),
      username: new FormControl(
        {
          value: isEmpty(this.data) ? '' : this.data?.name,
          disabled: !!this.data
        },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(this.MAX_LENGTH)
          ]
        }
      ),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(this.MAX_LENGTH)
        ]
      })
    });
    if (this.treeSelection.type !== ResourceType.OpenStack) {
      this.formGroup.get('username').clearValidators();
      this.formGroup.get('password').clearValidators();
    }
    this.formGroup.valueChanges.subscribe(res => {
      this.isTest = false;
    });
  }

  updateData() {
    if (!this.item) {
      return;
    }
    const showData = JSON.parse(this.item.extendInfo?.vdcNames).map(item => {
      return { name: item };
    });
    this.tableData = {
      data: showData,
      total: size(showData)
    };
    this.formGroup.get('children').setValue(this.tableData.data);
  }
  initConfig() {
    this.title =
      this.treeSelection.type === ResourceType.OpenStack
        ? this.i18n.get('common_domain_label')
        : this.i18n.get('common_cloud_platform_tenant_label');
    this.managerLabel =
      this.treeSelection.type === ResourceType.OpenStack
        ? this.i18n.get('protection_domain_user_label')
        : this.i18n.get('common_project_manager_label');
    this.showTips = this.treeSelection.type !== ResourceType.OpenStack;

    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_new_add_label'),
        onClick: () => {
          this.add();
        }
      }
    ];

    this.optItems = getPermissionMenuItem(opts);

    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_username_label')
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'modify',
                label: this.i18n.get('common_modify_label'),
                onClick: ([data]) => {
                  this.add(data);
                }
              },
              {
                id: 'delete',
                label: this.i18n.get('common_remove_label'),
                onClick: data => {
                  this.delete(data);
                }
              }
            ]
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: cols,
        compareWith: 'uuid',
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        showTotal: true
      }
    };
  }

  getTenantOptions(recordsTemp?, startPage?) {
    const isOpenstack = this.treeSelection.type === ResourceType.OpenStack;
    const conditions = {};
    if (isOpenstack) {
      assign(conditions, {
        subType: ResourceType.OpenStackDomain,
        type: ResourceType.StackDomain,
        rootUuid: this.treeSelection.rootUuid
      });
    } else {
      assign(conditions, {
        path: [['=~'], this.ip || this.treeSelection.path || ''],
        type: ResourceType.TENANT
      });
    }
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        const tenantArray = [];
        each(recordsTemp, item => {
          if (!isEmpty(this.item) || item.extendInfo?.visible === '0') {
            tenantArray.push({
              ...item,
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true
            });
          }
        });

        this.telnetOptions = tenantArray;
        return;
      }
      this.getTenantOptions(recordsTemp, startPage);
    });
  }

  test() {
    const selectTelnet = this.telnetOptions.find(
      item => item.uuid === this.formGroup.value.platform_telnet
    );
    this.tenant = selectTelnet.name;
    this.domainId = selectTelnet?.extendInfo?.domainId;
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.treeSelection.uuid
      })
      .subscribe(res => {
        const params = this.getParamsOpenstack(res);
        this.protectedEnvironmentApiService
          .CheckEnvironment({
            checkEnvironmentRequestBody: params as any,
            akOperationTips: false
          })
          .subscribe((response: any) => {
            const returnRes = JSON.parse(response);
            const idx = returnRes.findIndex(item => item.code !== 0);
            if (idx !== -1) {
              this.messageService.error(this.i18n.get(returnRes[idx].code), {
                lvMessageKey: 'errorKey',
                lvShowCloseButton: true
              });
            } else {
              this.messageService.success(
                this.i18n.get('common_test_success_label'),
                {
                  lvMessageKey: 'successKey',
                  lvShowCloseButton: true
                }
              );
              this.isTest = true;
              this.ok();
            }
          });
      });
  }

  ok() {
    const newItem = {
      name: this.formGroup.get('username').value,
      passwd: this.formGroup.value.password
    };
    this.globalService.emitStore({
      action: 'add-tenant-user',
      state: newItem
    });
  }

  add(data?) {
    if (!this.formGroup.value.platform_telnet) {
      this.formGroup.get('platform_telnet').markAsTouched();
      return;
    }
    const selectTelnet = this.telnetOptions.find(
      item => item.uuid === this.formGroup.value.platform_telnet
    );
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-telnet',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: isEmpty(data)
          ? this.i18n.get('common_new_add_label')
          : this.i18n.get('common_modify_label'),
        lvContent: AddUserComponent,
        lvComponentParams: {
          treeSelection: this.treeSelection,
          tenant: selectTelnet.name,
          domainId: selectTelnet?.extendInfo?.domainId,
          data
        },
        lvOkDisabled: true
      })
    );
  }

  delete(data) {
    const currentTableData = cloneDeep(this.tableData.data);
    remove(currentTableData, item => isEqual(data[0], item));
    this.tableData = {
      data: currentTableData,
      total: size(currentTableData)
    };
    this.formGroup.get('children').setValue(currentTableData);
    this.formGroup.get('children').updateValueAndValidity();
  }

  getParamsOpenstack(data) {
    // 用于test获取参数
    const params = {
      name: data.name,
      uuid: data.uuid,
      type: data.type,
      subType: data.subType,
      endpoint: data.endpoint,
      auth: {
        authType: data.auth.authType + '',
        authKey: this.formGroup.get('username').value,
        authPwd: this.formGroup.value.password
      },
      dependencies: {
        agents: data.dependencies.agents.map(item => {
          return { uuid: item.uuid };
        })
      },
      extendInfo: {
        domain: this.tenant,
        isVdc: 'true',
        domainId: this.domainId
      }
    };
    if (this.treeSelection?.type === ResourceType.OpenStack) {
      delete params.extendInfo.isVdc;
      delete params.extendInfo.domain;
      set(params.extendInfo, 'domainName', this.tenant);
      set(params.extendInfo, 'isDomain', 'true');
    }
    return params;
  }

  getParams() {
    const selectTelnet = this.telnetOptions.find(
      item => item.uuid === this.formGroup.value.platform_telnet
    );
    if (this.treeSelection.type === ResourceType.OpenStack) {
      return {
        name: selectTelnet.name,
        parentUuid: this.treeSelection.uuid,
        parentName: this.treeSelection.name,
        sourceType: 'register',
        extendInfo: {
          vdcNames: JSON.stringify(this.tableData.data.map(item => item.name)),
          visible: '1'
        },
        auth: {
          authKey: this.tableData.data[0]?.name,
          authPwd: this.tableData.data[0]?.passwd
        }
      };
    }
    return {
      name: selectTelnet.name,
      parentUuid: this.treeSelection.uuid,
      parentName: this.treeSelection.name,
      sourceType: 'register',
      extendInfo: {
        vdcNames: JSON.stringify(this.tableData.data.map(item => item.name)),
        visible: '1'
      },
      auth: {
        extendInfo: {
          vdcInfos: JSON.stringify(this.tableData.data)
        }
      }
    };
  }

  onOK() {
    const params = this.getParams();

    return this.protectedResourceApiService.UpdateResource({
      resourceId: this.formGroup.value.platform_telnet,
      UpdateResourceRequestBody: params
    });
  }
}
