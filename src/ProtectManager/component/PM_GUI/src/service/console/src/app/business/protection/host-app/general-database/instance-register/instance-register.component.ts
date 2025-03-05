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
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  getMultiHostOps,
  getPermissionMenuItem,
  HostService,
  I18NService,
  MODAL_COMMON,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  findIndex,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isEqual,
  isNil,
  isUndefined,
  keys,
  map,
  reject,
  remove,
  size
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { AddHostComponent } from './add-host/add-host.component';
import { cacheGuideResource } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-instance-register',
  templateUrl: './instance-register.component.html',
  styleUrls: ['./instance-register.component.less']
})
export class InstanceRegisterComponent implements OnInit {
  item;
  showPwd;
  pwdType = 'password';
  agentType;
  dataDetail;
  sourceType;
  optsConfig;
  optItems = [];
  hostOptions = [];
  scriptOptions = [];
  originalTableData;
  dataMap = DataMap;
  formGroup: FormGroup;
  tableConfig: TableConfig;
  readonly ONLINE_STATUS = '1';
  requireCustomParams = false;
  customParamsPlaceholder = this.i18n.get(
    'protection_custon_params_placeholder_label'
  );

  databaseAuthTip = this.i18n.get(
    'protection_general_db_auth_extendinfo_tips_label'
  );

  clusterOptions = this.dataMapService
    .toArray('generalDbClusterType')
    .map(item => {
      item['isLeaf'] = true;
      return item;
    })
    .filter(item => item.value !== DataMap.generalDbClusterType.single.value);
  authOptions = this.dataMapService
    .toArray('Database_Auth_Method')
    .map(item => {
      item['isLeaf'] = true;
      return item;
    });
  databaseTypeOptions = this.dataMapService
    .toArray('generalDbClusterType')
    .map(item => {
      item['isLeaf'] = true;
      return item;
    });
  tableData = {
    data: [],
    total: 0
  };

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  textAreaErrorTips = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [500])
  };
  agentsErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_host_number_least_2_label'),
    invalidStatus: this.i18n.get('protection_host_offline_tips_label')
  };

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private drawModalService: DrawModalService,
    private dataMapService: DataMapService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private hostService: HostService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initConfig();
    this.getHostOptions();
    this.agentType =
      this.sourceType === DataMap.Resource_Type.generalInstance.value
        ? this.i18n.get('protection_database_instance_label')
        : this.i18n.get('common_database_label');
  }

  copy() {
    return false;
  }

  authHelpHover() {
    const url =
      this.formGroup.get('verifyScript').value === 'gbase8a'
        ? `/console/assets/help/a8000/${
            this.i18n.isEn
              ? 'en-us/index.html#GBase_8a_00012.html'
              : 'zh-cn/index.html#GBase_8a_00011.html'
          }`
        : `/console/assets/help/a8000/${
            this.i18n.isEn
              ? 'en-us/index.html#SAP_HANA_0013.html'
              : 'zh-cn/index.html#SAP_HANA_00013.html'
          }`;
    this.appUtilsService.openSpecialHelp(url);
  }

  updateData() {
    if (!this.item) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.item.uuid })
      .subscribe(res => {
        this.dataDetail = res;

        if (res?.extendInfo?.firstClassification === '1') {
          if (res?.extendInfo?.instanceClassification === '1') {
            this.formGroup.patchValue({
              type: DataMap.Instance_Type.single.value,
              name: res.name,
              agents: first(map(get(res, 'dependencies.hosts'), 'uuid')),
              authMode: res?.auth?.authType,
              userName: get(res, 'auth.authKey'),
              verifyScript: res?.extendInfo?.script,
              customParams: res?.extendInfo?.customParams
            });
          } else {
            const children = map(get(res, 'dependencies.children'), item => {
              const child = cloneDeep(item);
              return child;
            });

            this.formGroup.patchValue({
              type: DataMap.Instance_Type.cluster.value,
              name: res.name,
              cluster: res?.extendInfo?.deployType,
              verifyScript: res?.extendInfo?.script,
              customParams: res?.extendInfo?.customParams,
              children: children
            });

            this.tableData = {
              data: children,
              total: size(children)
            };
          }
        } else {
          this.formGroup.patchValue({
            name: res.name,
            databaseType: res?.extendInfo?.deployType,
            authMode: res?.auth?.authType,
            userName: get(res, 'auth.authKey'),
            verifyScript: res?.extendInfo?.script,
            customParams: res?.extendInfo?.customParams
          });

          if (
            res?.extendInfo?.deployType ===
            DataMap.generalDbClusterType.single.value
          ) {
            this.formGroup
              .get('agents')
              .setValue(first(get(res, 'dependencies.hosts'))['uuid']);
            this.formGroup.get('agents').markAllAsTouched();
          } else {
            this.formGroup
              .get('databaseAgents')
              .setValue(map(get(res, 'dependencies.hosts'), 'uuid'));
            this.formGroup.get('databaseAgents').markAllAsTouched();
          }
        }
      });
  }

  private validHostStatus() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isEmpty(control.value) || !size(this.hostOptions)) {
        return null;
      }

      if (isArray(control.value)) {
        const selectHost = filter(this.hostOptions, item => {
          return includes(control.value, item.value);
        });

        return find(selectHost, item => {
          return item.linkStatus !== this.ONLINE_STATUS;
        })
          ? { invalidStatus: { value: control.value } }
          : null;
      } else {
        const selectHost = find(this.hostOptions, item => {
          return item.value === control.value;
        });
        return get(selectHost, 'linkStatus') !== this.ONLINE_STATUS
          ? { invalidStatus: { value: control.value } }
          : null;
      }
    };
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Instance_Type.single.value),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      databaseType: new FormControl(''),
      cluster: new FormControl(''),
      agents: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validHostStatus()
        ]
      }),
      databaseAgents: new FormControl([]),
      authMode: new FormControl(DataMap.Database_Auth_Method.os.value),
      userName: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(32)]
      }),
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(32)]
      }),
      authExtendInfo: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(500)]
      }),
      children: new FormControl([]),
      verifyScript: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      customParams: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(500)]
      })
    });

    if (this.sourceType === DataMap.Resource_Type.generalDatabase.value) {
      this.formGroup
        .get('databaseType')
        .setValidators([this.baseUtilService.VALID.required()]);
      this.formGroup
        .get('databaseType')
        .setValue(DataMap.generalDbClusterType.single.value);
    }

    if (!!this.item) {
      this.formGroup.get('name').disable();
    }

    this.watch();
  }

  watch() {
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.Instance_Type.single.value) {
        this.updateSingle();
      } else {
        this.updateCluster();
      }
      this.formGroup.get('cluster').updateValueAndValidity();
      this.formGroup.get('agents').updateValueAndValidity();
      this.formGroup.get('userName').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
      this.formGroup.get('children').updateValueAndValidity();
      this.formGroup.updateValueAndValidity();
    });
    this.formGroup.get('authMode').valueChanges.subscribe(res => {
      if (res === DataMap.Database_Auth_Method.db.value) {
        this.formGroup
          .get('userName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(32)
          ]);
        this.formGroup
          .get('password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(32)
          ]);
      } else {
        this.formGroup.get('userName').clearValidators();
        this.formGroup.get('password').clearValidators();
      }

      this.formGroup.get('userName').updateValueAndValidity();
      this.formGroup.get('password').updateValueAndValidity();
    });
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      if (!size(this.formGroup.value.children)) {
        return;
      }
    });

    this.formGroup.get('agents').valueChanges.subscribe(res => {
      this.getScriptOptions(res);
    });

    this.formGroup.get('databaseType').valueChanges.subscribe(res => {
      if (res === DataMap.generalDbClusterType.single.value) {
        this.formGroup
          .get('agents')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validHostStatus()
          ]);
        this.formGroup.get('databaseAgents').clearValidators();
      } else {
        this.formGroup
          .get('databaseAgents')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.minLength(2),
            this.validHostStatus()
          ]);
        this.formGroup.get('agents').clearValidators();
      }

      this.formGroup.get('agents').setValue('');
      this.formGroup.get('databaseAgents').setValue([]);
    });

    this.formGroup.get('databaseAgents').valueChanges.subscribe(res => {
      this.getScriptOptions(res);
    });

    this.formGroup.get('children').valueChanges.subscribe(res => {
      this.getScriptOptions(res);
    });

    this.formGroup.get('verifyScript').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      const authMode = get(
        find(this.scriptOptions, { value: res }),
        'supportType',
        []
      );
      const customParamsTemplate = get(
        find(this.scriptOptions, { value: res }),
        'customParamsTemplate'
      );
      const options = this.dataMapService
        .toArray('Database_Auth_Method')
        .map(item => {
          item['isLeaf'] = true;
          return item;
        });

      // 名称验证
      if (res === 'gbase8a') {
        this.databaseAuthTip = this.i18n.get(
          'protection_general_db_auth_extendinfo_tips_label',
          ['GBase 8a']
        );
        // gbase换校验方式
        this.formGroup
          .get('name')
          .setValidators([
            this.validGbaseName(),
            this.baseUtilService.VALID.required()
          ]);
        assign(this.nameErrorTip, {
          ...this.baseUtilService.nameErrorTip,
          invalidNameCombination: this.i18n.get(
            'protection_valid_client_name_label'
          ),
          invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [48]),
          invalidNameBegin: this.i18n.get(
            'system_valid_sftp_username_begin_label'
          )
        });
      } else {
        this.databaseAuthTip = this.i18n.get(
          'protection_general_db_auth_extendinfo_tips_label',
          ['SAP HANA']
        );
        this.formGroup
          .get('name')
          .setValidators([this.baseUtilService.VALID.name()]);
        assign(this.nameErrorTip, {
          ...this.baseUtilService.nameErrorTip,
          invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
        });
      }
      this.formGroup.get('name').updateValueAndValidity();

      // 自定义参数
      this.customParamsPlaceholder =
        customParamsTemplate ||
        this.i18n.get('protection_custon_params_placeholder_label');
      if (!!customParamsTemplate) {
        this.requireCustomParams = true;
        this.formGroup
          .get('customParams')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(500)
          ]);
      } else {
        this.requireCustomParams = false;
        this.formGroup
          .get('customParams')
          .setValidators([this.baseUtilService.VALID.maxLength(500)]);
      }
      this.formGroup.get('customParams').updateValueAndValidity();

      // 认证模式
      if (!!size(authMode)) {
        this.authOptions = options.filter(item =>
          includes(authMode, item.value)
        );

        if (isEmpty(this.item)) {
          this.formGroup.get('authMode').setValue(first(authMode));
        }
      } else {
        this.authOptions = options;
      }
    });
  }

  validGbaseName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || isEmpty(control.value)) {
        return null;
      }

      const reg_nameBegin = /^[a-zA-Z_]/;
      const reg_name = /^[a-zA-Z_0-9]+$/;
      const value = control.value;
      // 1、只能以字母或_开头。
      const reg1 = reg_nameBegin;
      if (!reg1.test(value)) {
        return { invalidNameBegin: { value: control.value } };
      }

      // 2、由字母、数字和“_”组成。
      const reg2 = reg_name;
      if (!reg2.test(value)) {
        return { invalidNameCombination: { value: control.value } };
      }

      // 3、长度范围是1到48位。
      const reg3 = /^.{1,48}$/;
      if (!reg3.test(value)) {
        return { invalidMaxLength: { value: control.value } };
      }

      return null;
    };
  }

  updateSingle() {
    this.formGroup
      .get('agents')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.validHostStatus()
      ]);
    this.formGroup.get('cluster').clearValidators();
    this.formGroup.get('children').clearValidators();
  }

  updateCluster() {
    this.formGroup
      .get('cluster')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('children')
      .setValidators([this.baseUtilService.VALID.minLength(1)]);
    this.formGroup.get('agents').clearValidators();
    this.formGroup.get('userName').clearValidators();
    this.formGroup.get('password').clearValidators();
    setTimeout(() => {
      this.dataTable.fetchData();
    }, 0);
  }

  showDbPwd() {
    this.showPwd = !this.showPwd;
    this.pwdType = this.showPwd ? 'text' : 'password';
  }

  getHostOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.generalDatabase.value}Plugin`]
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
        this.hostOptions =
          isEmpty(this.item) && !MultiCluster.isMulti
            ? filter(hostArray, item => {
                return (
                  item.linkStatus ===
                  DataMap.resource_LinkStatus_Special.normal.value
                );
              })
            : hostArray;
        this.updateData();
      }
    );
  }

  getScriptOptions(uuid) {
    this.scriptOptions = [];
    this.formGroup.get('verifyScript').setValue('');

    if (!uuid || !size(uuid)) {
      return;
    }

    const hostUuids =
      this.formGroup.value.type === DataMap.Instance_Type.cluster.value
        ? map(this.formGroup.value.children, 'uuid')
        : isArray(uuid)
        ? uuid
        : [uuid];
    const selectHost = filter(this.hostOptions, item => {
      return includes(hostUuids, item.value);
    });

    if (
      find(selectHost, item => {
        return item.linkStatus !== this.ONLINE_STATUS;
      })
    ) {
      return;
    }

    this.hostService
      .QueryAppConfigInfo({
        subType: DataMap.Resource_Type.generalDatabase.value,
        hostUuids: hostUuids
      })
      .subscribe(res => {
        const result = res;
        let scriptArray = [];
        const scripts = keys(result);

        each(scripts, item => {
          scriptArray.push({
            key: item,
            value: item,
            label: get(result, `${item}.databaseType`) || item,
            isLeaf: true,
            customParamsTemplate: get(
              result,
              `${item}.resource.customParamsTemplate`
            ),
            supportType: get(result, `${item}.resource.auth.supportType`)
          });
        });
        if (
          this.formGroup.get('databaseType').value ===
          DataMap.generalDbClusterType.sharding.value
        ) {
          scriptArray = filter(scriptArray, item => {
            return item.value !== 'saphana';
          });
        }
        // E6000不支持Gbase8a
        if (this.appUtilsService.isDistributed) {
          scriptArray = filter(scriptArray, item => {
            return item.value !== 'gbase8a';
          });
        }
        this.scriptOptions = scriptArray;

        if (!isEmpty(this.item)) {
          const resourceScript = find(this.scriptOptions, {
            value: get(this.item, 'extendInfo.script')
          });

          this.formGroup
            .get('verifyScript')
            .setValue(
              isNil(resourceScript) ? '' : get(resourceScript, 'value')
            );
        }
        return;
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
      }
    ];

    this.optsConfig = getPermissionMenuItem(opts);
    this.optItems = cloneDeep(reject(this.optsConfig, { id: 'add' }));

    const cols: TableCols[] = [
      {
        key: 'hostName',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'endpoint',
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
            maxDisplayItems: 0,
            items: [
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
        winTablePagination: true
      }
    };
  }

  add(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-host',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: !!data
          ? this.i18n.get('common_modify_label')
          : this.i18n.get('common_add_label'),
        lvContent: AddHostComponent,
        lvComponentParams: {
          parentUuid: this.formGroup.value.cluster,
          name: this.formGroup.get('name').value,
          children: this.formGroup.value.children,
          hostOptions: cloneDeep(this.hostOptions),
          rowData: data
        },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddHostComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res === 'INVALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddHostComponent;
            content.onOK().subscribe({
              next: res => {
                resolve(true);
                let currentTableData = cloneDeep(this.tableData.data);
                const modifiedIndex = findIndex(
                  currentTableData,
                  item =>
                    item.extendInfo?.hostId ===
                    get(first(content.data), 'extendInfo.hostId')
                );
                if (modifiedIndex !== -1) {
                  currentTableData[modifiedIndex] = cloneDeep(
                    first(content.data)
                  );
                  currentTableData = [...currentTableData];
                } else {
                  currentTableData = currentTableData.concat([...content.data]);
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

  getParams() {
    const originNode =
      this.formGroup.value.type === DataMap.Instance_Type.single.value
        ? get(this.dataDetail, 'dependencies.hosts')
        : get(this.dataDetail, 'dependencies.children');
    const currentNode =
      this.formGroup.value.type === DataMap.Instance_Type.cluster.value
        ? this.formGroup.value.children
        : this.formGroup.value.databaseType ===
          DataMap.generalDbClusterType.single.value
        ? [this.formGroup.value.agents]
        : this.formGroup.value.databaseAgents;
    const deletedNode = filter(
      originNode,
      item =>
        !find(currentNode, val => val === item.uuid || val.uuid === item.uuid)
    );

    return this.sourceType === DataMap.Resource_Type.generalDatabase.value
      ? {
          name: this.formGroup.get('name').value,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.generalDatabase.value,
          extendInfo: {
            firstClassification: '2',
            deployType: this.formGroup.value.databaseType,
            script: this.formGroup.value.verifyScript,
            customParams: this.formGroup.value.customParams
          },
          dependencies: {
            hosts: map(currentNode, item => {
              return { uuid: item };
            }),
            '-hosts': map(deletedNode, item => {
              return { uuid: item.uuid };
            })
          },
          auth: {
            authType: this.formGroup.value.authMode,
            authKey:
              this.formGroup.value.authMode ===
              DataMap.Database_Auth_Method.db.value
                ? this.formGroup.value.userName
                : '',
            authPwd:
              this.formGroup.value.authMode ===
              DataMap.Database_Auth_Method.db.value
                ? this.formGroup.value.password
                : '',
            extendInfo: {
              authCustomParams:
                this.formGroup.value.authMode ===
                DataMap.Database_Auth_Method.db.value
                  ? this.formGroup.value.authExtendInfo
                  : ''
            }
          }
        }
      : this.formGroup.value.type === DataMap.Instance_Type.single.value
      ? {
          name: this.formGroup.get('name').value,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.generalDatabase.value,
          extendInfo: {
            firstClassification: '1',
            instanceClassification: '1',
            deployType: '1',
            script: this.formGroup.value.verifyScript,
            customParams: this.formGroup.value.customParams
          },
          dependencies: {
            hosts: [{ uuid: this.formGroup.value.agents }],
            '-hosts': map(deletedNode, item => {
              return { uuid: item.uuid };
            })
          },
          auth: {
            authType: this.formGroup.value.authMode,
            authKey:
              this.formGroup.value.authMode ===
              DataMap.Database_Auth_Method.db.value
                ? this.formGroup.value.userName
                : '',
            authPwd:
              this.formGroup.value.authMode ===
              DataMap.Database_Auth_Method.db.value
                ? this.formGroup.value.password
                : '',
            extendInfo: {
              authCustomParams:
                this.formGroup.value.authMode ===
                DataMap.Database_Auth_Method.db.value
                  ? this.formGroup.value.authExtendInfo
                  : ''
            }
          }
        }
      : {
          name: this.formGroup.get('name').value,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.generalDatabase.value,
          extendInfo: {
            firstClassification: '1',
            instanceClassification: '2',
            deployType: this.formGroup.value.cluster,
            script: this.formGroup.value.verifyScript,
            customParams: this.formGroup.value.customParams
          },
          dependencies: {
            children: map(currentNode, item => {
              return {
                name: item.name,
                auth: item.auth,
                endpoint: item.endpoint,
                dependencies: item.dependencies,
                extendInfo: {
                  customParams: item.customParams
                }
              };
            }),
            '#children': map(deletedNode, item => {
              return {
                name: item.name,
                auth: item.auth,
                endpoint: item.endpoint,
                dependencies: item.dependencies,
                extendInfo: {
                  customParams: item.customParams
                }
              };
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
      if (!this.item) {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params as any
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
      } else {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.item.uuid,
            UpdateProtectedEnvironmentRequestBody: params
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
