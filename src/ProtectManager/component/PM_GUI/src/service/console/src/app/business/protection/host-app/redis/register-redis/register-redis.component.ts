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
import { Component, OnInit } from '@angular/core';
import {
  DataMapService,
  I18NService,
  BaseUtilService,
  CommonConsts,
  DataMap,
  MODAL_COMMON,
  ProtectedResourceApiService,
  ResourceType,
  ProtectedEnvironmentApiService,
  getPermissionMenuItem,
  OperateItems,
  MultiCluster,
  getMultiHostOps
} from 'app/shared';
import {
  isNumber,
  each,
  assign,
  isEmpty,
  size,
  isArray,
  values,
  cloneDeep,
  first,
  isEqual,
  remove,
  isUndefined,
  get,
  filter
} from 'lodash';
import {
  TableConfig,
  Filters,
  TableCols
} from 'app/shared/components/pro-table';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { RedisClusterNodeComponent } from './redis-cluster-node/redis-cluster-node.component';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { Observable } from 'rxjs';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-register-redis',
  templateUrl: './register-redis.component.html',
  styleUrls: ['./register-redis.component.less']
})
export class RegisterRedisComponent implements OnInit {
  item;
  okLoading = false;
  testLoading = false;
  proxyOptions = [];
  formGroup: FormGroup;
  usernameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  optItems = [];
  tableConfig: TableConfig;
  tableData = {
    data: [],
    total: 0
  };
  dataMap = DataMap;

  constructor(
    private appUtilsService: AppUtilsService,
    private i18n: I18NService,
    private fb: FormBuilder,
    private protectedResourceApiService: ProtectedResourceApiService,
    private drawModalService: DrawModalService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private virtualScroll: VirtualScrollService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateData(); // 回显数据
    this.getProxyOptions();
  }

  updateData() {
    if (!this.item) {
      return;
    }
    const showData = this.item.dependencies.children.map(item => {
      return {
        hostname: item.dependencies.agents[0].name,
        endpoint: item.extendInfo?.ip,
        status: item.extendInfo?.status,
        clientPath: item.extendInfo?.clientPath,
        ip: item.extendInfo?.ip,
        port: item.extendInfo?.port,
        hostUuid: item.dependencies.agents[0].uuid,
        authKey: item.auth.authKey,
        authType: item.auth.authType,
        authPwd: item.auth.authPwd,
        kerberosId: item.extendInfo?.kerberosId,
        uuid: item.uuid,
        sslEnable: item.extendInfo?.sslEnable === 'true'
      };
    });

    this.tableData = {
      data: showData,
      total: size(showData)
    };

    this.formGroup.get('children').setValue(showData);
    this.formGroup.get('children').updateValueAndValidity();
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.Redis.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            item.environment.extendInfo.scenario ===
            DataMap.proxyHostType.external.value
        );
        if (MultiCluster.isMulti && isEmpty(this.item)) {
          resource = getMultiHostOps(resource);
        }
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
      }
    );
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
          this.delete(first(data));
        }
      }
    };
    this.optItems = cloneDeep(getPermissionMenuItem(values(opts)));

    this.formGroup = this.fb.group({
      redisName: new FormControl(this.item ? this.item.name : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      children: new FormControl([])
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
        name: this.i18n.get('common_business_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'operation',
        width: 130,
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
        async: false,
        virtualScroll: true,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        colDisplayControl: false,
        fetchData: (filters: Filters) => {
          this.getData(filters);
        },
        trackByFn: (index, item) => {
          return item.resource_id;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  // 获取表格数据
  getData(filters: Filters) {
    let nodes = this.tableData.data;
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

  // 添加集群节点
  showAddPage(data: any = {}) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-cluster-node',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: isEmpty(data)
          ? this.i18n.get('common_add_label')
          : this.i18n.get('common_modify_label'),
        lvContent: RedisClusterNodeComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data,
          tableData: this.tableData,
          children: this.formGroup.value.children
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as RedisClusterNodeComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as RedisClusterNodeComponent;
            content.onOK().subscribe({
              next: (res: any) => {
                if (!res) {
                  resolve(false);
                  return;
                }
                resolve(true);
                let currentTableData = cloneDeep(this.tableData.data);
                if (isEmpty(data)) {
                  currentTableData = currentTableData.concat([content.addData]);
                } else {
                  const oldItem = currentTableData.find(
                    item => item.ip === data.ip && item.port === data.port
                  );
                  assign(oldItem, content.addData);
                }
                this.formGroup.get('children').setValue(currentTableData);
                this.formGroup.get('children').updateValueAndValidity();
                this.tableData = {
                  data: currentTableData,
                  total: size(currentTableData)
                };
              }
            });
          });
        }
      })
    );
  }

  delete(data) {
    const currentTableData = cloneDeep(this.tableData.data);
    remove(currentTableData, item => isEqual(data, item));
    this.tableData = {
      data: currentTableData,
      total: size(currentTableData)
    };
    this.formGroup.get('children').setValue(currentTableData);
    this.formGroup.get('children').updateValueAndValidity();
  }

  getParams() {
    const params = {
      name: this.formGroup.value.redisName,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.Redis.value,
      extendInfo: {
        type: 'cluster',
        resourceType: 'cluster'
      },
      dependencies: {
        children: this.formGroup.value.children.map(item => {
          const originExtendInfo = {};
          if (item.authType === 5) {
            originExtendInfo['kerberosId'] = item.kerberosId;
          }
          return {
            type: ResourceType.DATABASE,
            subType: DataMap.Resource_Type.Redis.value,
            uuid: !isEmpty(this.item) ? item.uuid : null,
            name: item.endpoint.split('.').join('_') + `_${item.port}`, // 采用IP+Port
            auth: {
              authType: item.authType,
              extendInfo: {
                ...originExtendInfo
              }
            },
            extendInfo: {
              port: item.port,
              ip: item.ip,
              resourceType: 'node',
              type: 'node',
              clientPath: item.clientPath,
              sslEnable: item.sslEnable,
              ...originExtendInfo
            },
            dependencies: {
              agents: [
                {
                  uuid: item.hostUuid,
                  endpoint: item.endpoint,
                  name: item.hostname
                }
              ]
            }
          };
        })
      }
    };

    if (this.item) {
      params['uuid'] = this.item.uuid;
    }

    return params;
  }

  onOK(): Observable<any> {
    const params = this.getParams();
    if (this.item) {
      return this.protectedEnvironmentApiService.UpdateProtectedEnvironment({
        UpdateProtectedEnvironmentRequestBody: params as any,
        envId: this.item.uuid
      });
    }
    return this.protectedEnvironmentApiService.RegisterProtectedEnviroment({
      RegisterProtectedEnviromentRequestBody: params as any
    });
  }
}
