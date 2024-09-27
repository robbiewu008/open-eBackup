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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { OptionItem } from '@iux/live';
import { StorResourceNodeComponent } from 'app/business/protection/cloud/huawei-stack/register-huawei-stack/store-resource-node/store-resource-node.component';
import {
  BaseUtilService,
  ClientManagerApiService,
  ClusterEnvironment,
  CommonConsts,
  DataMap,
  DataMapService,
  Features,
  getMultiHostOps,
  I18NService,
  MODAL_COMMON,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  RoleOperationMap,
  Scene
} from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isEqual,
  isNumber,
  map,
  omit,
  remove,
  size,
  trim
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { distinctUntilChanged } from 'rxjs/operators';

@Component({
  selector: 'aui-oracle-register',
  templateUrl: './register.component.html',
  styleUrls: ['./register.component.less']
})
export class RegisterComponent implements OnInit {
  rowData: any;
  formGroup: FormGroup;
  optItems = [];
  USER_MAX_LENGTH = 32;
  PATH_MAX_LENGTH = 2048;
  MAX_STORAGE_RESOURCES = 2; // 1.6版本存储资源至多只有2个
  dataMap = DataMap;
  hostOptions: OptionItem[] = [];
  clusterOptions: OptionItem[] = [];
  osType = ''; // 选中主机/集群的操作系统类型
  singleRunningConfig = false;
  clusterRunningConfig = false;
  hiddenStorage = this.appUtilsService.isHcsUser;
  isSupport = false;
  isDisplay = false; // 修改场景首次回显状态值
  tableConfig: TableConfig;
  tableData: TableData = {
    data: [],
    total: 0
  };
  databaseAuthOptions = this.dataMapService
    .toArray('Database_Auth_Method')
    .map(item => {
      item.isLeaf = true;
      return item;
    });
  asmAuthOptions = this.dataMapService.toArray('asmAuthMethod').map(item => {
    item.isLeaf = true;
    return item;
  });
  clusterNodes;

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_name_label'),
    invalidSameName: this.i18n.get('common_duplicate_name_label')
  };
  userErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.USER_MAX_LENGTH
    ])
  };
  nodeErrorTip = {
    required: this.i18n.get('common_host_number_least_2_label'),
    invalidMinLength: this.i18n.get('common_host_number_least_2_label')
  };

  pathErrorTip = {
    invalidName: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.PATH_MAX_LENGTH
    ])
  };
  roleOperationMap = RoleOperationMap;

  @ViewChild('inputUserNameTpl', { static: true })
  inputUserNameTpl: TemplateRef<any>;
  @ViewChild('inputPasswordTpl', { static: true })
  inputPasswordTpl: TemplateRef<any>;
  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit(): void {
    this.isDisplay = !!this.rowData;
    this.initForm();
    this.initConfig();
    this.getHost();
    this.patchForm();
    this.getCluster();
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.oracleType.single.value),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      host: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      cluster: new FormControl(''),
      authMethod: new FormControl(DataMap.Database_Auth_Method.os.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      databaseInstallName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(this.USER_MAX_LENGTH)
        ]
      }),
      databaseUsername: new FormControl(''),
      databasePassword: new FormControl(''),
      oracleHome: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath, false),
          this.baseUtilService.VALID.maxLength(this.PATH_MAX_LENGTH)
        ]
      }),
      oracleBase: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath, false),
          this.baseUtilService.VALID.maxLength(this.PATH_MAX_LENGTH)
        ]
      }),
      enableAsm: new FormControl(false),
      asmAuthMethod: new FormControl(DataMap.asmAuthMethod.os.value),
      asmInstallName: new FormControl(''),
      asmUsername: new FormControl(''),
      asmPassword: new FormControl(''),
      singleRunningUsername: new FormControl(''),
      singleRunningPassword: new FormControl(''),
      clusterRunningIps: new FormArray([]),
      children: new FormControl([])
    });
    this.listenForm();
  }

  get clusterRunningIps() {
    return (this.formGroup.get('clusterRunningIps') as FormArray).controls;
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'ipList',
        name: this.i18n.get('common_management_ip_label'),
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
        key: 'username',
        name: this.i18n.get('common_username_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
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
                  this.addStorage(data);
                }
              },
              {
                id: 'delete',
                label: this.i18n.get('common_delete_label'),
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
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: 10,
        pageSizeOptions: [10]
      }
    };
  }

  listenForm() {
    this.formGroup.get('type').valueChanges.subscribe(res => {
      this.isSupport = false;
      // op服务化需要隐藏存储资源
      this.hiddenStorage = this.appUtilsService.isHcsUser;
      if (res === DataMap.oracleType.single.value) {
        this.formGroup
          .get('host')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('cluster').setValue('');
        this.formGroup.get('cluster').clearValidators();
        if (this.singleRunningConfig) {
          this.formGroup
            .get('singleRunningUsername')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup.get('singleRunningUsername').clearValidators();
        }
      } else {
        this.formGroup
          .get('cluster')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('host').setValue('');
        this.formGroup.get('host').clearValidators();
        this.formGroup.get('singleRunningUsername').clearValidators();
      }
      this.formGroup.get('host').updateValueAndValidity();
      this.formGroup.get('cluster').updateValueAndValidity();
      this.formGroup.get('singleRunningUsername').updateValueAndValidity();
    });
    this.formGroup.get('host').valueChanges.subscribe(res => {
      this.disableStorage(res);
      // 单机数据库修改时不允许修改主机
      if (!res) {
        this.singleRunningConfig = false;
        return;
      }

      this.isSupportFunc(res);

      if (!!this.rowData) return;
      // 只有注册时才会去options里查询对应的主机
      const targetHost = find(this.hostOptions, { uuid: res });
      this.singleRunningConfig =
        !!targetHost && targetHost.osType === DataMap.Os_Type.windows.value;
      if (
        this.singleRunningConfig &&
        this.formGroup.value.type === DataMap.oracleType.single.value
      ) {
        this.formGroup.get('singleRunningPassword').setValue('');
        this.formGroup.get('singleRunningUsername').setValue('');
        this.formGroup
          .get('singleRunningUsername')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('singleRunningUsername').clearValidators();
      }
      this.formGroup.get('singleRunningUsername').updateValueAndValidity();
    });
    this.formGroup
      .get('cluster')
      .valueChanges.pipe(distinctUntilChanged())
      .subscribe(res => {
        this.disableStorage(res);
        if (!res) {
          this.clusterRunningConfig = false;
          return;
        }
        this.getIPByCluster(res);
      });
    this.formGroup.get('authMethod').valueChanges.subscribe(res => {
      if (res === DataMap.Database_Auth_Method.db.value) {
        this.formGroup
          .get('databaseUsername')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(this.USER_MAX_LENGTH)
          ]);
        this.formGroup
          .get('databasePassword')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(this.USER_MAX_LENGTH)
          ]);
      } else {
        this.formGroup.get('databaseUsername').clearValidators();
        this.formGroup.get('databasePassword').clearValidators();
      }
      this.formGroup.get('databaseUsername').updateValueAndValidity();
      this.formGroup.get('databasePassword').updateValueAndValidity();
    });
    this.formGroup.get('enableAsm').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('asmInstallName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(this.USER_MAX_LENGTH)
          ]);
        if (
          this.formGroup.value.asmAuthMethod === DataMap.asmAuthMethod.asm.value
        ) {
          this.formGroup
            .get('asmUsername')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(this.USER_MAX_LENGTH)
            ]);
          this.formGroup
            .get('asmPassword')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(this.USER_MAX_LENGTH)
            ]);
        } else {
          this.formGroup.get('asmUsername').clearValidators();
          this.formGroup.get('asmPassword').clearValidators();
        }
      } else {
        this.formGroup.get('asmInstallName').clearValidators();
        this.formGroup.get('asmUsername').clearValidators();
        this.formGroup.get('asmPassword').clearValidators();
      }
      this.formGroup.get('asmInstallName').updateValueAndValidity();
      this.formGroup.get('asmUsername').updateValueAndValidity();
      this.formGroup.get('asmPassword').updateValueAndValidity();
    });
    this.formGroup.get('asmAuthMethod').valueChanges.subscribe(res => {
      if (res === DataMap.asmAuthMethod.asm.value) {
        this.formGroup
          .get('asmUsername')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(this.USER_MAX_LENGTH)
          ]);
        this.formGroup
          .get('asmPassword')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(this.USER_MAX_LENGTH)
          ]);
      } else {
        this.formGroup.get('asmUsername').clearValidators();
        this.formGroup.get('asmPassword').clearValidators();
      }
      this.formGroup.get('asmUsername').updateValueAndValidity();
      this.formGroup.get('asmPassword').updateValueAndValidity();
    });
  }

  // 判断当前版本是否支持添加存储资源
  isSupportFunc(agent) {
    if (isEmpty(agent)) {
      return;
    }

    const params = {
      hostUuidsAndIps: isArray(agent) ? agent : [agent],
      applicationType: 'Oracle',
      scene: Scene.Register,
      buttonNames: [Features.StorageResources]
    };

    // 注册时单机场景下主机切换需要清空存储资源数据，修改时不允许修改主机
    // 集群只有修改首次回显时不清空该值
    if (
      (!this.rowData &&
        this.formGroup.get('type').value === DataMap.oracleType.single.value) ||
      (this.formGroup.get('type').value === DataMap.oracleType.cluster.value &&
        !this.isDisplay)
    ) {
      this.tableData = {
        data: [],
        total: 0
      };
      this.formGroup.get('children').setValue([]);
    }

    this.clientManagerApiService
      .queryAgentApplicationUsingPOST({
        AgentCheckSupportParam: params,
        akOperationTips: false
      })
      .subscribe(res => {
        this.isSupport = res.StorageResources;
      });
  }

  disableStorage(uuid: string) {
    const disabledOsType = DataMap.Os_Type.aix.value;
    let resources = [];
    if (this.rowData) {
      this.hiddenStorage =
        this.rowData?.environment?.osType === disabledOsType ||
        this.appUtilsService.isHcsUser;
      this.osType = this.rowData?.environment?.osType;
      return;
    }
    if (!uuid) {
      // 切换单机/集群时 host/cluster选项会被清空，此时osType为空
      this.osType = '';
      return;
    }
    if (this.formGroup.get('type').value === DataMap.oracleType.single.value) {
      resources = this.hostOptions;
    } else {
      resources = this.clusterOptions;
    }
    const targetOsType = String(
      find(resources, { uuid })?.osType
    ).toLowerCase();
    this.osType = targetOsType;
    this.hiddenStorage =
      targetOsType === disabledOsType || this.appUtilsService.isHcsUser;
  }

  patchForm() {
    if (!this.rowData) {
      return;
    }
    const params = {
      type: this.rowData.subType,
      name: this.rowData.name,
      host: this.rowData.environment?.uuid,
      cluster: this.rowData.environment?.uuid,
      authMethod: this.rowData.auth?.authType,
      databaseInstallName: this.rowData.extendInfo?.installUsername,
      enableAsm: !isEmpty(this.rowData.auth?.extendInfo?.asmInfo),
      oracleHome: this.rowData.extendInfo?.accessOracleHome || '',
      oracleBase: this.rowData.extendInfo?.accessOracleBase || ''
    };
    if (this.rowData.auth?.authType === DataMap.Database_Auth_Method.db.value) {
      assign(params, {
        databaseUsername: this.rowData.auth?.authKey
      });
    }
    if (!isEmpty(this.rowData.auth?.extendInfo?.asmInfo)) {
      assign(params, {
        asmInstallName: this.rowData.auth?.extendInfo?.asmInfo.installUsername,
        asmAuthMethod: this.rowData.auth?.extendInfo?.asmInfo.authType,
        asmUsername: this.rowData.auth?.extendInfo?.asmInfo.authKey
      });
    }
    if (this.rowData.subType) {
      assign(params, {
        node: map(this.rowData?.dependencies?.agents, 'uuid')
      });
    }
    if (this.rowData.environment.osType === DataMap.Os_Type.linux.value) {
      this.formGroup.get('oracleHome').disable();
      this.formGroup.get('oracleBase').disable();
    }

    if (this.rowData.environment.osType === DataMap.Os_Type.windows.value) {
      const runUserInfo = JSON.parse(
        get(this.rowData.extendInfo, 'runUserInfo', '[]')
      );
      // 集群数据库的runUserInfo回显无法实现
      if (this.rowData.subType === DataMap.oracleType.single.value) {
        this.singleRunningConfig = true;
        assign(params, {
          singleRunningUsername: get(first(runUserInfo), 'low_auth_user', '')
        });
        this.formGroup
          .get('singleRunningUsername')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('singleRunningUsername').updateValueAndValidity();
      }
    }
    this.formGroup.patchValue(params);
    this.updateTableData();
  }

  updateTableData() {
    const showData = JSON.parse(this.rowData.extendInfo?.storages || '[]').map(
      item => {
        return {
          ipList: item.ipList,
          port: item.port,
          username: item.username,
          enableCert: item.enableCert,
          transport_protocol: item.transport_protocol
        };
      }
    );
    this.tableData = {
      data: showData,
      total: size(showData)
    };
    this.formGroup.get('children').setValue(this.tableData.data);
  }

  addStorage(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-cluster-node',
        lvWidth: MODAL_COMMON.normalWidth + 150,
        lvHeader: isEmpty(data)
          ? this.i18n.get('common_add_label')
          : this.i18n.get('common_modify_label'),
        lvContent: StorResourceNodeComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data,
          tableData: this.tableData,
          subType: DataMap.Resource_Type.oracle.value,
          children: this.formGroup.value.children
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as StorResourceNodeComponent;
            content.onOK().subscribe({
              next: res => {
                if (!res) {
                  resolve(false);
                  return;
                }
                resolve(true);
                let currentTableData = cloneDeep(this.tableData.data);
                if (isEmpty(data)) {
                  currentTableData = currentTableData.concat([content.data]);
                } else {
                  const oldItem = currentTableData.find(
                    item =>
                      String(item.ipList) === String(data.ipList) &&
                      String(item.port) === String(data.port)
                  );
                  assign(oldItem, content.data);
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
    remove(currentTableData, item => isEqual(data[0], item));
    this.tableData = {
      data: currentTableData,
      total: size(currentTableData)
    };
    this.formGroup.get('children').setValue(currentTableData);
    this.formGroup.get('children').updateValueAndValidity();
  }

  getHost() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.oracle.value}Plugin`]
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
        if (MultiCluster.isMulti) {
          resource = getMultiHostOps(resource);
        } else {
          resource = filter(
            resource,
            item =>
              item.environment.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
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
        this.hostOptions = hostArray;
      }
    );
  }

  getCluster(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: 200,
        pageNo: startPage || CommonConsts.PAGE_START,
        conditions: JSON.stringify({
          subType: [ClusterEnvironment.oralceClusterEnv],
          linkStatus: [DataMap.resource_LinkStatus_Special.normal.value]
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          this.clusterOptions = map(recordsTemp, item => {
            return assign(item, {
              key: item.uuid,
              value: item.uuid,
              label: item.name + `(${item.endpoint})`,
              isLeaf: true
            });
          });
          defer(() => (this.isDisplay = false));
          return;
        }
        this.getCluster(recordsTemp, startPage);
      });
  }

  addForm(data?) {
    const ipsArr = this.formGroup.get('clusterRunningIps') as FormArray;
    ipsArr.clear();
    this.formGroup.get('clusterRunningIps').updateValueAndValidity();
    // 不需要填写runUserInfo时清除所有校验
    if (!this.clusterRunningConfig) return;
    each(data, item => {
      ipsArr.push(
        this.fb.group({
          ip: item?.endpoint || item?.ip,
          lowAuthUser: new FormControl(item?.low_auth_user || '', {
            validators: [this.baseUtilService.VALID.required()]
          }),
          password: ''
        })
      );
    });
  }

  getIPByCluster(clusterId: string) {
    if (
      !clusterId ||
      this.formGroup.value.type !== DataMap.oracleType.cluster.value
    )
      return;
    this.protectedResourceApiService
      .ShowResource({ resourceId: clusterId })
      .subscribe(
        (res: any) => {
          this.clusterRunningConfig =
            res.osType === DataMap.Os_Type.windows.value;
          const agents = res.dependencies?.agents;
          this.clusterNodes = agents;
          this.addForm(agents);
          const arr = map(agents, 'uuid');
          this.isSupportFunc(arr);
        },
        error => {
          this.clusterRunningConfig = false;
          this.addForm();
        }
      );
  }

  getParams() {
    const params = {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: this.formGroup.value.type,
      parentUuid: this.formGroup.value.host,
      dependencies: {
        agents: [{ uuid: this.formGroup.value.host }]
      },
      extendInfo: {
        hostId: this.formGroup.value.host,
        installUsername: this.formGroup.value.databaseInstallName,
        accessOracleHome: this.formGroup.get('oracleHome').value,
        accessOracleBase: this.formGroup.get('oracleBase').value
      },
      auth: {
        authType: this.formGroup.value.authMethod,
        authKey:
          this.formGroup.value.authMethod ===
          DataMap.Database_Auth_Method.db.value
            ? this.formGroup.value.databaseUsername
            : '',
        authPwd:
          this.formGroup.value.authMethod ===
          DataMap.Database_Auth_Method.db.value
            ? this.formGroup.value.databasePassword
            : '',
        extendInfo: {}
      }
    };
    let authRunUserInfo = [];
    let extendRunUserInfo = [];
    if (
      this.formGroup.value.type === DataMap.oracleType.single.value &&
      this.singleRunningConfig
    ) {
      const {
        host,
        singleRunningUsername,
        singleRunningPassword
      } = this.formGroup.value;
      const selectHost = find(this.hostOptions, { uuid: host });
      const ip = selectHost?.endpoint;
      const tmpObj = { ip, low_auth_user: trim(singleRunningUsername) };
      if (selectHost?.extendInfo?.subNetFixedIp) {
        assign(tmpObj, {
          subNetFixedIp: selectHost?.extendInfo?.subNetFixedIp
        });
      }
      extendRunUserInfo = [tmpObj];
      authRunUserInfo = [{ ...tmpObj, password: trim(singleRunningPassword) }];
    }
    if (
      this.formGroup.value.type === DataMap.oracleType.cluster.value &&
      this.clusterRunningConfig
    ) {
      const ipArr = (this.formGroup.get('clusterRunningIps') as FormArray)
        .value;
      each(ipArr, item => {
        const tmpObj = {
          ip: item.ip,
          low_auth_user: trim(item.lowAuthUser)
        };
        const node = find(
          this.clusterNodes,
          n => n.endpoint === item.ip || n.ip === item.ip
        );
        if (node?.extendInfo?.subNetFixedIp) {
          assign(tmpObj, {
            subNetFixedIp: node?.extendInfo?.subNetFixedIp
          });
        }
        extendRunUserInfo.push(tmpObj);
        authRunUserInfo.push({
          ...tmpObj,
          password: trim(item.password)
        });
      });
    }
    assign(params.auth.extendInfo, {
      runUserPwd: JSON.stringify(authRunUserInfo)
    });
    assign(params.extendInfo, {
      runUserInfo: JSON.stringify(extendRunUserInfo)
    });
    if (this.formGroup.value.enableAsm) {
      assign(params.auth.extendInfo, {
        asmInfo: JSON.stringify({
          installUsername: this.formGroup.value.asmInstallName,
          authType: this.formGroup.value.asmAuthMethod,
          authKey:
            this.formGroup.value.asmAuthMethod ===
            DataMap.asmAuthMethod.asm.value
              ? this.formGroup.value.asmUsername
              : '',
          authPwd:
            this.formGroup.value.asmAuthMethod ===
            DataMap.asmAuthMethod.asm.value
              ? this.formGroup.value.asmPassword
              : ''
        })
      });
    }
    if (this.formGroup.value.type === DataMap.oracleType.cluster.value) {
      assign(params, {
        parentUuid: this.formGroup.value.cluster
      });
      delete params.dependencies;
    }
    if (size(this.formGroup.value.children) > 0) {
      let password: string[] = [];
      const storageParams = map(
        cloneDeep(this.formGroup.value.children),
        item => {
          password.push(item.password);
          return omit(item, ['storageType', 'parent', 'password']);
        }
      );
      assign(params.extendInfo, {
        storages: JSON.stringify(
          map(storageParams, item => {
            return {
              username: item.username,
              port: item.port,
              ipList: item.ipList,
              enableCert: String(+item.enableCert),
              certName: item.certName,
              certSize: item.certSize,
              crlName: item.crlName,
              crlSize: item.crlSize,
              transport_protocol: item.transport_protocol,
              device_type: item.device_type
            };
          })
        )
      });
      assign(params.auth.extendInfo, {
        storages: JSON.stringify(storageParams),
        storagesPwd: JSON.stringify(password)
      });
    } else {
      // 删除的情况storages下发空串
      assign(params.extendInfo, {
        storages: ''
      });
      assign(params.auth.extendInfo, {
        storages: ''
      });
    }
    return params;
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  showGuideClusterNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const params = this.getParams();
      if (this.rowData) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowData?.uuid,
            UpdateResourceRequestBody: params
          })
          .subscribe({
            next: res => {
              observer.next(res);
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      } else {
        this.protectedResourceApiService
          .CreateResource({
            CreateResourceRequestBody: params
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next(res);
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
    });
  }
}
