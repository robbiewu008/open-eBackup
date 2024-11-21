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
import { Component, OnInit, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { AddNodeComponent } from 'app/business/protection/database/ant-db/register/add-node/add-node.component';
import {
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  InstanceType,
  MODAL_COMMON,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import {
  cacheGuideResource,
  USER_GUIDE_CACHE_DATA
} from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  find,
  first,
  includes,
  isEmpty,
  isEqual,
  map,
  omit,
  pick,
  remove,
  set,
  size,
  some,
  trim,
  uniq
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register',
  templateUrl: './register.component.html',
  styleUrls: ['./register.component.less']
})
export class RegisterComponent implements OnInit {
  rowData;
  optItems = [];
  hostOptions = [];
  clusterOptions = [];
  dataMap = DataMap;
  hostNum = 0;
  _isEmpty = isEmpty;
  tableData = {
    data: [],
    total: 0
  };
  tableConfig: TableConfig;
  formGroup: FormGroup;
  extendsAuth: { [key: string]: any } = {};
  cols: TableCols[] = [];
  extraCols: TableCols[];
  clusterInfo; // 缓存集群信息
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip
  };
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32]),
    invalidUserNameBegin: this.i18n.get(
      'common_valid_linux_user_name_begin_label'
    ),
    invalidUserName: this.i18n.get('common_valid_linux_user_name_label')
  };
  dataBaseErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidNameBegin: this.i18n.get('system_valid_sftp_username_begin_label'),
    invalidDataBaseName: this.i18n.get('common_valid_database_name_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [63])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  // 路径校验
  pathErrorTip = {
    ...this.baseUtilService.filePathErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024]),
    invalidSpecailChars: this.i18n.get('common_valid_file_path_label'),
    pathError: this.i18n.get('common_path_error_label')
  };

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public baseUtilService: BaseUtilService,
    public dataMapService: DataMapService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private clientManagerApiService: ClientManagerApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initConfig(); // 只做表格初始化操作
    this.updateData(); // 回显数据
    this.getHostOptions();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  showClusterGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  updateData() {
    if (!this.rowData) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe((res: any) => {
        const data =
          res.subType === DataMap.Resource_Type.AntDBInstance.value
            ? {
                name: res.name,
                type: DataMap.Instance_Type.single.value,
                agents: first(map(res.dependencies.agents, 'uuid')),
                userName: res.extendInfo.osUsername,
                client: res.extendInfo.clientPath,
                business_ip: res.extendInfo.serviceIp,
                port: res.extendInfo.instancePort,
                database_username: res.auth.authKey
              }
            : {
                name: res.name,
                type: DataMap.Instance_Type.cluster.value,
                client: res.extendInfo.clientPath,
                port: res.extendInfo.instancePort,
                database_username: res.auth.authKey,
                databaseStreamUserName: res.auth.dbStreamRepUser,
                userName: res.extendInfo.osUsername,
                children: res.dependencies.children
              };
        this.formGroup.patchValue(data);
        this.updateTableData(res, data);
      });
  }

  private updateTableData(res, data) {
    if (res.subType === DataMap.Resource_Type.AntDBClusterInstance.value) {
      data?.children.filter(item => {
        assign(item, {
          hostName: item.dependencies?.agents[0]?.name,
          ip: item.dependencies?.agents[0]?.endpoint,
          port: item.extendInfo?.instancePort,
          business_ip: item.extendInfo?.serviceIp,
          client: item.extendInfo?.clientPath,
          adbhamgrPath: item.extendInfo?.adbhamgrPath
        });
      });
      setTimeout(() => {
        this.tableData = {
          data: data?.children,
          total: size(data?.children)
        };
      }, 1000);
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Instance_Type.single.value),
      name: new FormControl(
        { value: '', disabled: !!this.rowData },
        {
          validators: [this.baseUtilService.VALID.name()]
        }
      ),
      agents: new FormControl('', [this.baseUtilService.VALID.required()]),
      port: new FormControl(
        { value: '6655', disabled: !!this.rowData },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]
        }
      ),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      client: new FormControl({ value: '', disabled: !!this.rowData }, [
        this.baseUtilService.VALID.required(),
        this.validPath()
      ]),
      business_ip: new FormControl({ value: '', disabled: !!this.rowData }, [
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.ip()
      ]),
      database_username: new FormControl('', [
        this.validDataBaseName(),
        this.baseUtilService.VALID.maxLength(63),
        this.baseUtilService.VALID.required()
      ]),
      database_password: new FormControl('', [
        this.baseUtilService.VALID.required()
      ]),
      databaseStreamUserName: new FormControl(''),
      databaseStreamPassword: new FormControl(''),
      children: new FormControl([])
    });
    this.watch();
  }

  validDataBaseName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const name = control.value;
      if (!CommonConsts.REGEX.opengaussRestoreName.test(name)) {
        return { invalidNameBegin: { value: control.value } };
      }
      if (!CommonConsts.REGEX.dataBaseName.test(name)) {
        return { invalidDataBaseName: { value: control.value } };
      }
      return null;
    };
  }

  validUserName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const userName = control.value;
      if (!CommonConsts.REGEX.linuxUserNameBegin.test(userName)) {
        return { invalidUserNameBegin: { value: control.value } };
      }
      if (!CommonConsts.REGEX.linuxUserName.test(userName)) {
        return { invalidUserName: { value: control.value } };
      }
      return null;
    };
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      const paths = control.value.split(',')?.filter(item => {
        return !isEmpty(item);
      });
      if (paths.length !== uniq(paths).length) {
        return { samePathError: { value: control.value } };
      }

      if (
        find(paths, path => {
          return !CommonConsts.REGEX.templatLinuxPath.test(path);
        }) ||
        find(paths, path => {
          return path.length > 1024;
        })
      ) {
        return { pathError: { value: control.value } };
      }
    };
  }

  getProxyOptions(uuid) {
    if (!uuid) {
      return;
    }
    const params = {
      resourceId: uuid
    };
    this.protectedResourceApiService
      .ShowResource(params)
      .subscribe((res: any) => {
        this.hostNum = res.dependencies.agents.length;
        this.clusterInfo = res;
      });
  }

  watch() {
    const controlArr = [
      'agents',
      'business_ip',
      'client',
      'children',
      'databaseStreamUserName',
      'databaseStreamPassword'
    ];
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.Instance_Type.single.value) {
        this.updateSingle();
      } else {
        this.updateCluster();
      }
      each(controlArr, control =>
        this.formGroup.get(control).updateValueAndValidity()
      );
    });
  }

  updateSingle() {
    this.formGroup
      .get('agents')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('business_ip')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.ip()
      ]);
    this.formGroup
      .get('client')
      .setValidators([this.baseUtilService.VALID.required(), this.validPath()]);
    this.formGroup.get('children').clearValidators();
    this.formGroup.get('databaseStreamUserName').clearValidators();
    this.formGroup.get('databaseStreamPassword').clearValidators();
  }

  updateCluster() {
    if (!this.rowData) {
      this.formGroup
        .get('children')
        .setValidators([this.baseUtilService.VALID.minLength(1)]);
    }
    this.formGroup
      .get('databaseStreamUserName')
      .setValidators([
        this.validDataBaseName(),
        this.baseUtilService.VALID.maxLength(63),
        this.baseUtilService.VALID.required()
      ]);
    this.formGroup
      .get('databaseStreamPassword')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup.get('agents').clearValidators();
    this.formGroup.get('business_ip').clearValidators();
    this.formGroup.get('client').clearValidators();
  }

  getHostOptions() {
    const extParams = {
      conditions: JSON.stringify({
        pluginType: `${DataMap.Resource_Type.AntDBInstance.value}Plugin`
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        const hostArray = [];
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          });
        });
        this.hostOptions = [...hostArray];
      }
    );
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_add_label'),
        onClick: () => {
          this.add();
        },
        disableCheck: () => {
          return !isEmpty(this.rowData);
        }
      }
    ];

    this.optItems = getPermissionMenuItem(opts);

    this.cols = [
      {
        key: 'hostName',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'client',
        name: this.i18n.get('common_database_client_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'adbhamgrPath',
        name: this.i18n.get('common_config_file_full_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'business_ip',
        name: this.i18n.get('common_dataplane_ip_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'port',
        name: this.i18n.get('common_database_port_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'operation',
        width: '70px',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'modify',
                label: this.i18n.get('common_modify_label'),
                disableCheck: () => {
                  return !isEmpty(this.rowData);
                },
                onClick: ([data]) => {
                  this.add(data);
                }
              },
              {
                id: 'delete',
                label: this.i18n.get('common_remove_label'),
                disableCheck: () => {
                  return !isEmpty(this.rowData);
                },
                onClick: data => {
                  this.delete(data);
                }
              }
            ]
          }
        }
      }
    ];
    this.extraCols = [
      {
        key: 'ip',
        name: this.i18n.get('common_ip_address_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: this.cols,
        compareWith: 'business_ip',
        colDisplayControl: true,
        trackByFn: (index, item) => {
          return item.business_ip;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  add(item?) {
    const formValue = pick(this.formGroup.value, [
      'name',
      'userName',
      'database_username',
      'database_password',
      'databaseStreamUserName',
      'databaseStreamPassword'
    ]);
    if (some(formValue, (value, key) => isEmpty(value))) {
      this.formGroup.markAllAsTouched();
      return;
    }
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-ant-db-node',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: isEmpty(item)
          ? this.i18n.get('common_add_label')
          : this.i18n.get('common_modify_label'),
        lvContent: AddNodeComponent,
        lvComponentParams: {
          parentFormGroupValue: this.formGroup.value,
          hostOptions: this.hostOptions,
          item
        },
        lvOkDisabled: isEmpty(item),
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddNodeComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res === 'INVALID';
          });
        },
        lvOk: modal => {
          return this.afterAddNode(modal, item);
        }
      })
    );
  }

  private afterAddNode(modal, item) {
    return new Promise(resolve => {
      const content = modal.getContentComponent() as AddNodeComponent;
      content.onOK().subscribe({
        next: res => {
          resolve(true);
          let currentTableData = cloneDeep(this.tableData.data);
          if (isEmpty(item)) {
            currentTableData = currentTableData.concat([content.data]);
          } else {
            const oldItem = currentTableData.find(
              cur => cur.extendInfo?.hostId === item.extendInfo?.hostId
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

  getParams() {
    this.extendsAuth.authType = DataMap.Postgre_Auth_Method.db.value;
    this.extendsAuth.authKey = this.formGroup.value.database_username;
    this.extendsAuth.authPwd = this.formGroup.value.database_password;
    return this.formGroup.value.type === DataMap.Instance_Type.single.value
      ? {
          name: this.formGroup.get('name').value,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.AntDBInstance.value,
          parentUuid: this.formGroup.value.agents,
          extendInfo: {
            hostId: this.formGroup.value.agents,
            instancePort: this.formGroup.get('port').value,
            clientPath: this.formGroup.get('client').value,
            serviceIp: this.formGroup.get('business_ip').value,
            osUsername: this.formGroup.value.userName,
            isTopInstance: InstanceType.TopInstance
          },
          dependencies: {
            agents: [{ uuid: this.formGroup.value.agents }]
          },
          auth: {
            ...this.extendsAuth,
            extendInfo: {}
          }
        }
      : {
          name: this.formGroup.get('name').value,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.AntDBClusterInstance.value,
          parentUuid: '',
          extendInfo: {
            osUsername: this.formGroup.value.userName,
            isTopInstance: InstanceType.TopInstance
          },
          auth: {
            ...this.extendsAuth,
            extendInfo: {
              dbStreamRepUser: this.formGroup.value.databaseStreamUserName,
              dbStreamRepPwd: this.formGroup.value.databaseStreamPassword
            }
          },
          dependencies: {
            children: this.formGroup.value.children.map(item => {
              const newItem = omit(item, [
                'port',
                'hostName',
                'ip',
                'client',
                'business_ip',
                'parent',
                'adbhamgrPath'
              ]);
              set(newItem, 'auth', {
                ...this.extendsAuth,
                extendInfo: {
                  dbStreamRepUser: this.formGroup.value.databaseStreamUserName,
                  dbStreamRepPwd: this.formGroup.value.databaseStreamPassword
                }
              });
              set(
                newItem,
                'extendInfo.osUsername',
                this.formGroup.value.userName
              );
              return newItem;
            })
          }
        };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.rowData) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowData.uuid,
            UpdateResourceRequestBody: params
          })
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
        return;
      }
      this.protectedResourceApiService
        .CreateResource({
          CreateResourceRequestBody: params as any
        })
        .subscribe({
          next: res => {
            cacheGuideResource(res);
            observer.next();
            observer.complete();
          },
          error: err => {
            observer.error(err);
            observer.complete();
          }
        });
    });
  }
}
