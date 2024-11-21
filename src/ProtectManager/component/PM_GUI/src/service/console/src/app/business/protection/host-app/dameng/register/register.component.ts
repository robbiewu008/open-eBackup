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
import { ChangeDetectorRef, Component, OnInit, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  MultiCluster,
  getMultiHostOps
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';

import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  includes,
  isEmpty,
  isNumber,
  isUndefined,
  map,
  reject,
  size,
  trim,
  get
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { first } from 'rxjs/operators';
import { AddInstanceComponent } from './add-instance/add-instance.component';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-register',
  templateUrl: './register.component.html',
  styleUrls: ['./register.component.less']
})
export class RegisterComponent implements OnInit {
  data;
  isModify = false;

  optsConfig;
  selectHost;
  optItems = [];
  hostOptions = [];
  dataMap = DataMap;
  authOptions = this.dataMapService
    .toArray('Database_Auth_Method')
    .filter(item => {
      return (item.isLeaf = true);
    });
  pwdComplexTipLabel = this.i18n.get(
    'common_dameng_register_possword_tips_label'
  );
  tableConfig: TableConfig;
  tableData: TableData;
  formGroup: FormGroup;
  deleteData = [];

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  passwordErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [48]),
    unsupportValueError: this.i18n.get('common_dameng_password_error_label')
  };

  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };

  validInstance = new Subject();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private messageService: MessageService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.isModify = !!this.data;
    this.initForm();
    this.initConfig();
    this.updateData();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      subType: new FormControl(DataMap.Resource_Type.Dameng_singleNode.value),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      agents: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      port: new FormControl('5236', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ],
        updateOn: 'change'
      }),
      auth_method: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      database_username: new FormControl(''),
      database_password: new FormControl('')
    });

    this.watch();
  }

  watch() {
    this.formGroup.get('subType').valueChanges.subscribe(res => {
      if (res === DataMap.Resource_Type.Dameng_singleNode.value) {
        this.validInstance.next(true);
        this.formGroup
          .get('agents')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('port')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]);
        this.formGroup
          .get('auth_method')
          .setValidators([this.baseUtilService.VALID.required()]);
        if (
          this.formGroup.value.auth_method ===
          this.dataMap.Database_Auth_Method.db.value
        ) {
          this.formGroup
            .get('database_username')
            .setValidators([
              this.baseUtilService.VALID.name(),
              this.baseUtilService.VALID.required()
            ]);
          this.formGroup
            .get('database_password')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.vaildPasswords(),
              this.baseUtilService.VALID.maxLength(48)
            ]);
        } else {
          this.formGroup.get('database_username').clearValidators();
          this.formGroup.get('database_password').clearValidators();
        }
      } else {
        this.validInstance.next(this.tableData?.total > 0);
        this.formGroup.get('agents').clearValidators();
        this.formGroup.get('port').clearValidators();
        this.formGroup.get('auth_method').clearValidators();
        this.formGroup.get('database_username').clearValidators();
        this.formGroup.get('database_password').clearValidators();
      }
      this.formGroup.get('agents').updateValueAndValidity();
      this.formGroup.get('port').updateValueAndValidity();
      this.formGroup
        .get('auth_method')
        .updateValueAndValidity({ emitEvent: false });
      this.formGroup.get('database_username').updateValueAndValidity();
      this.formGroup.get('database_password').updateValueAndValidity();
    });

    this.formGroup.get('auth_method').valueChanges.subscribe(res => {
      if (res === this.dataMap.Database_Auth_Method.db.value) {
        this.formGroup
          .get('database_username')
          .setValidators([
            this.baseUtilService.VALID.name(),
            this.baseUtilService.VALID.required()
          ]);
        this.formGroup
          .get('database_password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.vaildPasswords(),
            this.baseUtilService.VALID.maxLength(48)
          ]);
      } else {
        this.formGroup.get('database_username').clearValidators();
        this.formGroup.get('database_password').clearValidators();
      }
      this.formGroup.get('database_username').updateValueAndValidity();
      this.formGroup.get('database_password').updateValueAndValidity();
    });
  }

  vaildPasswords(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const password = control.value;
      if (includes(password, ' ')) {
        return { unsupportValueError: { value: control.value } };
      }
      return null;
    };
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.Dameng_cluster.value}Plugin`],
        environment: {
          linkStatus: [['=='], 1]
        }
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            item.environment.extendInfo.scenario ===
            DataMap.proxyHostType.external.value
        );
        if (MultiCluster.isMulti && isEmpty(this.data)) {
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
        this.hostOptions = hostArray;
      }
    );
  }

  updateData() {
    if (!this.data) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.data.uuid
      })
      .subscribe((res: any) => {
        const params = {};
        if (res.subType === DataMap.Resource_Type.Dameng_singleNode.value) {
          assign(params, {
            subType: res.subType,
            name: res.name,
            port: res.extendInfo.port,
            agents: res.dependencies.agents[0].uuid,
            auth_method: res.auth.authType
          });
          if (res.auth.authType === 2) {
            assign(params, {
              database_username: res.auth.authKey
            });
          }
        } else {
          assign(params, {
            subType: res.subType,
            name: res.name
          });
          const angetTables = map(res.dependencies.children, item => {
            return {
              id: item.uuid,
              name: item.dependencies.agents[0].name,
              ip: item.dependencies.agents[0].endpoint,
              port: item.extendInfo.port,
              authType: item.auth.authType,
              authKey: item.auth.authKey,
              authPwd: item.auth.authPwd,
              uuid: item.dependencies.agents[0].uuid
            };
          });
          this.tableData = {
            data: angetTables,
            total: size(angetTables)
          };
        }
        this.formGroup.patchValue(params);
      });
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_add_label'),
        onClick: () => {
          this.add();
        }
      },
      {
        id: 'delete',
        type: 'primary',
        label: this.i18n.get('common_delete_label'),
        onClick: data => {
          this.remove(data[0]);
        }
      }
    ];
    this.optItems = cloneDeep(reject(opts, { id: 'add' }));
    this.optsConfig = getPermissionMenuItem(
      cloneDeep(reject(opts, { id: 'delete' }))
    );
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'ip',
        name: this.i18n.get('common_ip_address_label'),
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
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 2,
            items: this.optItems
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: cols,
        compareWith: 'id',
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  add() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-host-dameng',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('common_add_label'),
        lvContent: AddInstanceComponent,
        lvComponentParams: { hostOptions: this.hostOptions },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddInstanceComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(status => {
            modalIns.lvOkDisabled = status !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddInstanceComponent;
            content.onOK().subscribe(res => {
              this.selectHost = filter(
                this.hostOptions,
                item => item.uuid === res.host
              );
              const parmas = {
                name: this.selectHost[0].name,
                ip: this.selectHost[0].endpoint,
                port: res.port,
                uuid: res.host,
                id: res.id,
                authType: res.auth_method,
                authKey: res.database_username,
                authPwd: res.database_password
              };
              resolve(true);
              this.validInstance.next(true);
              if (isEmpty(this.tableData)) {
                this.tableData = {
                  data: [parmas],
                  total: 1
                };
              } else {
                const repeat = find(this.tableData.data, item => {
                  return (
                    String(item.ip) === String(parmas.ip) &&
                    String(item.port) === String(parmas.port)
                  );
                });
                if (size(repeat)) {
                  this.messageService.error(
                    this.i18n.get('common_repeat_instance_tips_label')
                  );
                  this.tableData = {
                    data: [...this.tableData.data],
                    total: this.tableData.total
                  };
                } else {
                  this.tableData = {
                    data: [...this.tableData.data, parmas],
                    total: ++this.tableData.total
                  };
                }
              }
            });
          });
        }
      })
    );
  }

  remove(data) {
    const idx = this.tableData.data.findIndex(
      item => item.port === data.port && item.ip === data.ip
    );
    const deleteNode = this.tableData.data.splice(idx, 1);
    this.deleteData.push(deleteNode[0]);
    this.tableData = {
      data: [...this.tableData.data],
      total: size(this.tableData.data)
    };
    this.validInstance.next(size(this.tableData.data) > 0);
  }

  getParams(subType) {
    if (subType === DataMap.Resource_Type.Dameng_singleNode.value) {
      return {
        name: this.formGroup.value.name,
        type: ResourceType.DATABASE,
        subType: this.formGroup.value.subType,
        auth: {
          authType: this.formGroup.value.auth_method,
          authKey: this.formGroup.value.database_username,
          authPwd: this.formGroup.value.database_password
        },
        extendInfo: {
          isTopInstance: '1',
          port: this.formGroup.value.port
        },
        dependencies: {
          agents: [{ uuid: this.formGroup.value.agents }]
        }
      };
    } else {
      return {
        name: this.formGroup.value.name,
        type: ResourceType.DATABASE,
        subType: this.formGroup.value.subType,
        extendInfo: {
          isTopInstance: '1'
        },
        dependencies: {
          children: map(this.tableData.data, item => {
            return {
              name: item.name,
              type: ResourceType.DATABASE,
              subType: DataMap.Resource_Type.Dameng_singleNode.value,
              parentUuid: item.uuid,
              auth: {
                authType: item.authType,
                authKey: item.authKey,
                authPwd: item.authPwd
              },
              extendInfo: {
                isTopInstance: '0',
                port: item.port
              },
              dependencies: {
                agents: [{ uuid: item.uuid }]
              }
            };
          })
        }
      };
    }
  }

  updateParams(subType) {
    if (subType === DataMap.Resource_Type.Dameng_singleNode.value) {
      return {
        name: this.formGroup.value.name,
        type: ResourceType.DATABASE,
        subType: this.formGroup.value.subType,
        auth: {
          authType: this.formGroup.value.auth_method,
          authKey: this.formGroup.value.database_username,
          authPwd: this.formGroup.value.database_password
        },
        extendInfo: {
          isTopInstance: '1',
          port: this.formGroup.value.port
        },
        dependencies: {
          agents: [{ uuid: this.formGroup.value.agents }]
        }
      };
    } else {
      return {
        name: this.formGroup.value.name,
        type: ResourceType.DATABASE,
        subType: this.formGroup.value.subType,
        extendInfo: {
          isTopInstance: '1'
        },
        dependencies: {
          children: map(this.tableData.data, item => {
            return {
              uuid: item.id,
              name: item.name,
              type: ResourceType.DATABASE,
              subType: DataMap.Resource_Type.Dameng_singleNode.value,
              parentUuid: item.uuid,
              auth: {
                authType: item.authType,
                authKey: item.authKey,
                authPwd: item.authPwd
              },
              extendInfo: {
                isTopInstance: '0',
                port: item.port
              },
              dependencies: {
                agents: [{ uuid: item.uuid }]
              }
            };
          }),
          '-children': map(this.deleteData, item => {
            return {
              uuid: item.id
            };
          })
        }
      };
    }
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      let params = this.getParams(this.formGroup.value.subType);
      if (this.data) {
        params = this.updateParams(this.formGroup.value.subType);
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.data.uuid,
            UpdateProtectedEnvironmentRequestBody: params
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
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params as any
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
      }
    });
  }
}
