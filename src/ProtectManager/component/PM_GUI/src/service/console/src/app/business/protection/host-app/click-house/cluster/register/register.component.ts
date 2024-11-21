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
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { Component, OnInit, ViewChild, TemplateRef } from '@angular/core';
import {
  DataMapService,
  I18NService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  MODAL_COMMON,
  ProtectedResourceApiService,
  ResourceType,
  GlobalService,
  ProtectedEnvironmentApiService,
  getPermissionMenuItem,
  OperateItems
} from 'app/shared';
import { ModalRef } from '@iux/live';
import {
  assign,
  isEmpty,
  size,
  isArray,
  values,
  cloneDeep,
  first,
  unionBy,
  isNil,
  get
} from 'lodash';
import {
  TableConfig,
  Filters,
  TableCols
} from 'app/shared/components/pro-table';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { AddNodeComponent } from '../add-node/add-node.component';

@Component({
  selector: 'aui-register',
  templateUrl: './register.component.html',
  styleUrls: ['./register.component.less']
})
export class RegisterComponent implements OnInit {
  item;
  okLoading = false;
  testLoading = false;
  proxyOptions = [];
  formGroup: FormGroup;
  cloneData = [];

  usernameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;

  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  optItems = [];
  tableConfig: TableConfig;
  tableData: any = {};
  dataMap = DataMap;

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private fb: FormBuilder,
    private protectedResourceApiService: ProtectedResourceApiService,
    private drawModalService: DrawModalService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private virtualScroll: VirtualScrollService,
    private globalService: GlobalService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    // 修改功能
    if (this.item) {
      this.initTableData();
    }
    this.getFooter();
    this.initForm();
    this.getProxyOptions();

    this.globalService.getState('insertClickHouseCluster').subscribe(res => {
      this.tableData = cloneDeep(res);
      this.cloneData = this.tableData.data || [];
    });
  }

  initTableData() {
    const nodes = this.getShowTableData(this.item?.dependencies?.children); // 总数据
    this.tableData = {
      data: nodes,
      total: size(nodes)
    };
  }

  getShowTableData(data) {
    if (isNil(data)) return [];

    return data?.map(item => {
      const originExtendInfo = {};
      // kerber认证方式
      if (
        item.auth.authType ===
        this.dataMap.clickHouse_Auth_Method_Type.kerber.value
      ) {
        originExtendInfo['kerberosId'] = item.extendInfo?.kerberosId;
      }
      const authExtendInfo = {};

      // 数据库认证方式
      if (
        item.auth.authType ===
        this.dataMap.clickHouse_Auth_Method_Type.database.value
      ) {
        authExtendInfo['authKey'] = item.auth.authKey;
        authExtendInfo['authPwd'] = item.auth.authPwd;
      }

      return {
        hostname: item?.dependencies?.agents[0]?.name,
        endpoint: item?.dependencies?.agents[0]?.endpoint,
        status: item.extendInfo.status,
        clientPath: item.extendInfo.clientPath,
        ip: item.extendInfo.ip,
        port: item.extendInfo.port,
        role: item.extendInfo.role,
        slot: item.extendInfo.slot,
        authType: item.auth.authType,
        authKey: item.auth.authKey,
        authPwd: item.auth.authPwd,
        hostUuid: item.dependencies.agents[0].uuid,
        kerberosId: item.extendInfo?.kerberosId,
        clusterParams: {
          child: {
            uuid: item.uuid,
            type: ResourceType.NODE,
            subType: DataMap.Resource_Type.ClickHouse.value,
            name: item.name,
            environment: {
              uuid: item.environment.uuid,
              rootUuid: item.environment.rootUuid,
              endpoint: item.environment.endpoint,
              name: item.dependencies.agents[0].name,
              port: item.dependencies.agents[0].port
            },
            auth: {
              authType: item.auth.authType,
              ...originExtendInfo,
              extendInfo: {
                ...originExtendInfo
              }
            },
            extendInfo: {
              port: item.extendInfo.port,
              ip: item.extendInfo.ip,
              resourceType: 'node',
              type: 'node',
              clientPath: item.extendInfo.clientPath,
              ...originExtendInfo
            },
            dependencies: {
              agents: [
                {
                  uuid: item?.dependencies?.agents[0]?.uuid,
                  endpoint: item?.dependencies?.agents[0]?.endpoint,
                  name: item?.dependencies?.agents[0]?.name
                }
              ]
            }
          }
        }
      };
    });
  }

  getProxyOptions(recordsTemp?, startPage?) {
    if (isNil(this.item)) return;
    const params = {
      resourceId: this.item.uuid
    };
    this.protectedResourceApiService.ShowResource(params).subscribe(res => {
      const nodes = this.getShowTableData(
        get(res, ['dependencies', 'children'])
      ); // 总数据
      this.tableData = {
        data: nodes,
        total: size(nodes)
      };
    });
  }

  getFooter() {
    this.modal.setProperty({ lvFooter: this.footerTpl });
  }

  initForm() {
    const opts: { [key: string]: ProButton } = {
      modify: {
        id: 'modify',
        permission: OperateItems.RegisterRedis,
        label: this.i18n.get('common_modify_label'),
        onClick: data => {
          this.showAddPage(first(data));
        }
      },
      deleteResource: {
        id: 'deleteResource',
        permission: OperateItems.DeleteResource,
        label: this.i18n.get('common_delete_label'),
        onClick: data => {
          this.deleteItem(first(data));
        }
      }
    };
    this.optItems = cloneDeep(getPermissionMenuItem(values(opts)));

    this.formGroup = this.fb.group({
      clusterName: new FormControl(this.item ? this.item.name : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      })
    });

    const cols: TableCols[] = [
      {
        key: 'hostname',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'endpoint',
        width: 80,
        name: this.i18n.get('common_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Redis_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Redis_Status')
        }
      },
      {
        key: 'clientPath',
        name: this.i18n.get('common_client_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'ip',
        width: 80,
        name: this.i18n.get('common_business_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        width: 90,
        name: this.i18n.get('common_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'operation',
        width: 100,
        useOpWidth: true,
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: this.optItems
          }
        }
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'resource_id',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filters: Filters) => {
          this.getData(filters);
        },
        trackByFn: (_, item) => {
          return item.resource_id;
        }
      },
      pagination: {
        winTablePagination: true
      }
    };
  }

  // 获取表格数据
  getData(filters: Filters) {
    if (this.cloneData !== undefined) {
      this.tableData.data = this.cloneData;
    }

    let nodes; // 总数据
    if (!this.item) {
      nodes = this.tableData.data;
    } else {
      nodes = unionBy(
        this.cloneData,
        this.getShowTableData(this.item.dependencies.children),
        'uuid'
      );
    }
    let params;
    if (filters.conditions) {
      params = JSON.parse(filters.conditions);
      nodes = nodes.filter(item => {
        for (const key in params) {
          if (Object.prototype.hasOwnProperty.call(params, key)) {
            if (key === 'status' || key === 'role') {
              if (isArray(params[key])) {
                if (!params[key].includes(item[key])) {
                  return false;
                }
              }
            } else {
              if (item[key] !== params[key]) {
                return false;
              }
            }
          }
        }
        return true;
      });
    }
    this.tableData = {
      data: nodes,
      total: size(nodes)
    };
  }

  // 添加集群结点
  showAddPage(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-cluster-node',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: isEmpty(data)
          ? this.i18n.get('common_add_label')
          : this.i18n.get('common_modify_label'),
        lvContent: AddNodeComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data,
          tableData: this.tableData
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddNodeComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        }
      })
    );
  }

  deleteItem(data) {
    const idx = this.tableData.data.findIndex(
      item => item.port === data.port && item.ip === data.ip
    );
    this.tableData.data.splice(idx, 1);
    this.tableData = {
      data: [...this.tableData.data],
      total: size(data)
    };
  }
  getParams() {
    const params = {
      name: this.formGroup.value.clusterName,
      type: ResourceType.CLUSTER,
      subType: DataMap.Resource_Type.ClickHouse.value,
      extendInfo: {
        type: 'cluster',
        resourceType: 'cluster'
      },
      dependencies: {
        children: []
      }
    };

    if (this.item) {
      params['uuid'] = this.item.uuid;
    }

    for (const item of this.tableData.data) {
      params.dependencies.children.push(item.clusterParams.child);
    }

    return params;
  }

  ok() {
    if (this.formGroup.invalid) {
      return;
    }
    const params = this.getParams() as any;
    if (this.item) {
      this.protectedEnvironmentApiService
        .UpdateProtectedEnvironment({
          UpdateProtectedEnvironmentRequestBody: params,
          envId: this.item.uuid
        })
        .subscribe(() => {
          this.modal.close();
          this.globalService.emitStore({
            action: 'registerClickHouseCluster',
            state: params
          });
        });
    } else {
      this.protectedEnvironmentApiService
        .RegisterProtectedEnviroment({
          RegisterProtectedEnviromentRequestBody: params
        })
        .subscribe(() => {
          this.modal.close();
          this.globalService.emitStore({
            action: 'registerClickHouseCluster',
            state: params
          });
        });
    }
  }
}
