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
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  MessageService,
  ModalRef,
  UploadFile,
  UploadFileStatusEnum
} from '@iux/live';
import {
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  MultiCluster,
  getMultiHostOps,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  RouterUrl
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  defer,
  differenceBy,
  each,
  filter,
  first,
  get,
  includes,
  isEmpty,
  isEqual,
  map,
  remove,
  size,
  toString as _toString,
  trim,
  isArray
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { StorResourceNodeComponent } from './store-resource-node/store-resource-node.component';

@Component({
  selector: 'aui-register-huawei-stack-compute',
  templateUrl: './register-huawei-stack.component.html',
  styleUrls: ['./register-huawei-stack.component.less']
})
export class RegisterHuaWeiStackComponent implements OnInit {
  item;
  treeSelection;
  DataMap = DataMap;
  tableConfig: TableConfig;
  formGroup: FormGroup;
  pwdVisible = false;
  originNameList = [];
  originIpList = [];

  cinderCertFilters = [];

  selectCinderFile = '';
  optItems = [];
  proxyOptions = []; // 代理主机
  tableData = {
    data: [],
    total: 0
  };

  cinderFiles = [];

  cinderName = '';
  cinderSize = '';
  hostBuiltinLabel = this.i18n.get('protection_hcs_host_builtin_label');
  hostExternalLabel = this.i18n.get('protection_hcs_host_external_label');
  centralizedLabel = this.i18n.get('common_san_storage_label');
  scaleoutLabel = this.i18n.get('protection_database_type_block_storage_label');
  agentTips = this.i18n.get('protection_hcs_agents_link_tips_label');

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_name_label'),
    invalidSameName: this.i18n.get('common_duplicate_name_label')
  };
  domainNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_invalid_input_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  };
  userNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private modal: ModalRef,
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    public dataMapService: DataMapService,
    private message: MessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initConfig(); // 只做表格初始化操作
    this.initFilters();
    this.updateData(); // 回显数据
    this.getProxyOptions();
  }

  copy() {
    return false;
  }

  helpHover() {
    this.appUtilsService.openRouter(RouterUrl.ProtectionHostAppHost);
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(!isEmpty(this.item) ? this.item.name : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(),
          this.validIsSameName()
        ]
      }),
      ip: new FormControl(
        {
          value: !isEmpty(this.item) ? this.item.extendInfo?.ip : '',
          disabled: !!this.item
        },
        [this.baseUtilService.VALID.required(), this.baseUtilService.VALID.ip()]
      ),
      domain_name: new FormControl(
        !isEmpty(this.item) ? this.item.endpoint : '',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(255),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.nasshareDomain)
          ]
        }
      ),
      username: new FormControl(
        !isEmpty(this.item) ? this.item.auth.authKey : 'bss_admin',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]
        }
      ),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),

      agents: new FormControl(
        !isEmpty(this.item)
          ? this.item?.dependencies?.agents.map(item => item.uuid)
          : [],
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.minLength(1)
          ]
        }
      ),
      cert: new FormControl(
        !isEmpty(this.item)
          ? JSON.parse(this.item?.extendInfo?.enableCert || '{}')
          : true
      ),
      children: new FormControl([])
    });

    this.formGroup.statusChanges.subscribe(() => this.disableOkBtn());
    this.formGroup.get('cert')?.valueChanges.subscribe(res => {
      this.selectCinderFile = ''; // 清空cinder证书
      !res && this._cinderClear()._cinderClear();
    });
  }

  cinderFilesChange(files) {
    if (size(files) === 0) {
      each(['cinderName', 'cinderSize'], key => (this[key] = ''));
    } else {
      this.cinderName = get(first(files), 'name');
      this.cinderSize = get(first(files), 'fileSize');
    }
  }

  updateData() {
    if (!this.item) {
      return;
    }
    const showData = JSON.parse(this.item.extendInfo?.storages || '[]').map(
      item => {
        return {
          ip: item.ipList || item.ip,
          port: item.port,
          username: item.username,
          enableCert: item.enableCert,
          certName: item.certName,
          certSize: item.certSize,
          storageType:
            item.storageType === '1'
              ? this.scaleoutLabel
              : this.centralizedLabel,
          isVbsNodeInfo: item.isVbsNodeInfo,
          vbsNodeUserName: item.vbsNodeUserName,
          vbsNodeIp: item.vbsNodeIp,
          vbsNodePort: item.vbsNodePort
        };
      }
    );
    this.tableData = {
      data: showData,
      total: size(showData)
    };
    this.formGroup.get('children').setValue(this.tableData.data);

    const { extendInfo } = this.item;
    defer(() => {
      if (extendInfo.cinderName) {
        this.cinderFiles = [
          {
            key: '-1',
            name: extendInfo.cinderName,
            fileSize: extendInfo.cinderSize,
            status: UploadFileStatusEnum.SUCCESS
          }
        ];
        this.cinderName = extendInfo.cinderName;
        this.cinderSize = extendInfo.cinderSize;
      }
      this.formGroup.get('cert').setValue(extendInfo.enableCert === '1');
    });
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.value.cert
      ? this.formGroup.invalid || (!this.selectCinderFile && !this.cinderName)
      : this.formGroup.invalid;
  }

  initFilters() {
    this.cinderCertFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );

            this.selectCinderFile = ''; // 清空
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled = true;
            return '';
          }

          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectCinderFile = atob(
              (reader.result as any).replace('data:', '').replace(/^.+,/, '')
            );
            this.disableOkBtn();
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_add_label'),
        disableCheck: () => {
          return this.tableData?.total === 32;
        },
        disabledTips: this.i18n.get('protection_number_limited_label'),
        onClick: () => {
          this.showAddPage();
        }
      }
    ];

    this.optItems = getPermissionMenuItem(opts);

    const cols: TableCols[] = [
      {
        key: 'storageType',
        name: this.i18n.get('protection_hcs_storage_type_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'ip',
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
                  this.showAddPage(data);
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
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.uuid;
        }
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

  private _cinderClear() {
    this.cinderFiles = [];
    each(['selectCinderFile', 'cinderName', 'cinderSize'], key => {
      this[key] = '';
    });
    this.disableOkBtn();
    return this;
  }

  cinderChange(e) {
    e?.action === 'remove' && this._cinderClear();
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        pluginType: `HCScontainerPlugin`
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        // 创建不展示离线主机
        const hostArray = [];
        if (isEmpty(this.item)) {
          if (MultiCluster.isMulti) {
            resource = getMultiHostOps(resource, true);
          } else {
            resource = filter(resource, val => {
              return (
                val.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
              );
            });
          }
        }
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
        if (!isEmpty(this.formGroup.value.agents)) {
          this.formGroup
            .get('agents')
            .setValue(
              filter(this.formGroup.value.agents, item =>
                includes(map(hostArray, 'value'), item)
              )
            );
        }
      }
    );
  }

  validIsSameName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }

      return this.originNameList.includes(trim(control.value))
        ? { invalidSameName: { value: control.value } }
        : null;
    };
  }

  showAddPage(data?) {
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
          children: this.formGroup.value.children,
          subType: DataMap.Resource_Type.HCS.value
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
                      _toString(item.ip) === _toString(data.ip) &&
                      item.port === data.port
                  );
                  assign(oldItem, content.data, {
                    storageType:
                      content.data?.storageType === 0
                        ? this.centralizedLabel
                        : this.scaleoutLabel
                  });
                }
                this.formGroup.get('children').setValue(currentTableData);
                this.formGroup.get('children').updateValueAndValidity();
                this.tableData = {
                  data: currentTableData,
                  total: size(currentTableData)
                };
                if (isEmpty(data)) {
                  this.tableData.data[
                    this.tableData.data.length - 1
                  ].storageType =
                    this.tableData.data[this.tableData.data.length - 1]
                      .storageType === 0
                      ? this.centralizedLabel
                      : this.scaleoutLabel;
                }
              }
            });
          });
        }
      })
    );
  }

  getParams() {
    let reduceAgents = [];
    const params = cloneDeep(this.formGroup.value.children);
    if (this.formGroup.value.children.length !== 0) {
      params.forEach(item => {
        item.storageType =
          item.storageType === this.centralizedLabel ? '0' : '1';
      });
    }
    if (this.item) {
      reduceAgents = differenceBy(
        this.item?.dependencies?.agents.map(item => item.uuid),
        this.formGroup.value.agents
      );
    }
    return {
      name: this.formGroup.value.name,
      type: ResourceType.HCS,
      subType: ResourceType.HCS_CONTAINER,
      endpoint: this.formGroup.value.domain_name,
      extendInfo: {
        domain: 'mo_bss_admin',
        ip: this.formGroup.get('ip').value,
        storages: JSON.stringify(
          map(this.formGroup.value.children, item => {
            return {
              username: item.username,
              port: item.port,
              ip: isArray(item.ip) ? first(item.ip) : item.ip?.split(',')[0],
              ipList: isArray(item.ip) ? item.ip?.join(',') : item.ip,
              enableCert: String(+item.enableCert),
              certName: item.certName,
              certSize: item.certSize,
              crlName: item.crlName,
              crlSize: item.crlSize,
              storageType:
                item.storageType === this.centralizedLabel ? '0' : '1',
              isVbsNodeInfo: item.isVbsNodeInfo,
              vbsNodeUserName: item.vbsNodeUserName,
              vbsNodeIp: item.vbsNodeIp,
              vbsNodePort: item.vbsNodePort
            };
          })
        ),
        enableCert: String(+this.formGroup.value.cert),
        cinderName: this.formGroup.value.cert ? this.cinderName : '',
        cinderSize: this.formGroup.value.cert ? this.cinderSize : ''
      },
      auth: {
        authType: '1',
        authKey: this.formGroup.value.username,
        authPwd: this.formGroup.value.password,
        extendInfo: {
          enableCert: String(+this.formGroup.value.cert),
          cpsCertification: this.formGroup.value.cert
            ? this.selectCinderFile
            : '',
          storages: JSON.stringify(
            map(params, item => {
              assign(item, {
                ip: isArray(item.ip) ? first(item.ip) : item.ip?.split(',')[0],
                ipList: isArray(item.ip) ? item.ip?.join(',') : item.ip
              });
              return item;
            })
          )
        }
      },
      dependencies: {
        agents: map(this.formGroup.value.agents, item => {
          return {
            uuid: item
          };
        }),
        '-agents': !isEmpty(this.item)
          ? reduceAgents.map(item => {
              return { uuid: item };
            })
          : []
      }
    };
  }

  create(): Observable<void> {
    const params = this.getParams();
    return new Observable<void>((observer: Observer<void>) => {
      this.protectedEnvironmentApiService
        .RegisterProtectedEnviroment({
          RegisterProtectedEnviromentRequestBody: params as any
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }

  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.protectedEnvironmentApiService
        .UpdateProtectedEnvironment({
          UpdateProtectedEnvironmentRequestBody: this.getParams(),
          envId: this.item.uuid
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }

  // 存储资源删除表格数据
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
}
