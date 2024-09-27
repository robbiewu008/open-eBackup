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
  getPermissionMenuItem,
  I18NService,
  InstanceType,
  MODAL_COMMON,
  ProtectedResourceApiService,
  ResourceType,
  MultiCluster,
  getMultiHostOps
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
  find,
  first,
  isEmpty,
  isEqual,
  isNumber,
  map,
  omit,
  remove,
  size,
  trim,
  uniq,
  filter,
  includes
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { KingBaseAddHostComponent } from './add-host/king-base-add-host.component';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-king-base-register',
  templateUrl: './king-base-register.component.html',
  styleUrls: ['./king-base-register.component.less']
})
export class KingBaseRegisterComponent implements OnInit {
  item;
  optItems = [];
  hostOptions = [];
  clusterOptions = [];
  dataMap = DataMap;
  hostNum = 0;
  tableData = {
    data: [],
    total: 0
  };
  tableConfig: TableConfig;
  formGroup: FormGroup;
  extendsAuth = {};

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
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
  // 路径校验
  pathErrorTip = {
    ...this.baseUtilService.filePathErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024]),
    invalidSpecailChars: this.i18n.get('common_valid_file_path_label'),
    pathError: this.i18n.get('common_path_error_label')
  };

  // 认证方式
  authMethodOptions = this.dataMapService
    .toArray('postgre_Auth_Method_Type')
    .filter(v => (v.isLeaf = true));

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private protectedResourceApiService: ProtectedResourceApiService,
    public dataMapService: DataMapService,
    private drawModalService: DrawModalService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initConfig(); // 只做表格初始化操作
    this.updateData(); // 回显数据
    this.getHostOptions();
    this.getClusterOptions();
  }

  showHostGuideNew(item): boolean {
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
    if (!this.item) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.item.uuid })
      .subscribe(res => {
        const data =
          res.subType === DataMap.Resource_Type.KingBaseInstance.value
            ? {
                name: res.name,
                type: DataMap.Instance_Type.single.value,
                agents: first(map(res['dependencies']['agents'], 'uuid')),
                userName: res['extendInfo']['osUsername'],
                client: res['extendInfo']['clientPath'],
                business_ip: res['extendInfo']['serviceIp'],
                port: res['extendInfo']['instancePort'],
                auth_method: res['auth']['authType'],
                database_username: res['auth']['authKey']
              }
            : {
                name: res.name,
                type: DataMap.Instance_Type.cluster.value,
                cluster: res.parentUuid,
                client: res['extendInfo']['clientPath'],
                port: res['extendInfo']['instancePort'],
                userName: res['extendInfo']['osUsername'],
                children: res['dependencies']['children']
              };
        if (
          res.subType === DataMap.Resource_Type.KingBaseClusterInstance.value
        ) {
          data?.children.filter(item => {
            assign(item, {
              hostName: item.dependencies?.agents[0]?.name,
              ip: item.dependencies?.agents[0]?.endpoint,
              port: item.extendInfo?.instancePort,
              business_ip: item.extendInfo?.serviceIp,
              client: item.extendInfo?.clientPath
            });
          });
          this.tableData = {
            data: data?.children.map(item => ({
              ...item,
              userName: item.extendInfo.osUsername,
              databaseUserName: item.auth.authKey
            })),
            total: size(data?.children)
          };
          this.hostNum = data.children?.length;
        }

        this.formGroup.patchValue(data);
      });
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Instance_Type.single.value),
      name: new FormControl(
        { value: '', disabled: !!this.item },
        {
          validators: [
            this.baseUtilService.VALID.name(),
            this.baseUtilService.VALID.maxLength(64)
          ]
        }
      ),
      cluster: new FormControl(''),
      agents: new FormControl('', [this.baseUtilService.VALID.required()]),
      port: new FormControl(
        { value: '54321', disabled: !!this.item },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]
        }
      ),
      userName: new FormControl(
        { value: '', disabled: !!this.item },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(32)
          ]
        }
      ),
      client: new FormControl({ value: '', disabled: !!this.item }, [
        this.baseUtilService.VALID.required(),
        this.validPath()
      ]),
      business_ip: new FormControl({ value: '', disabled: !!this.item }, [
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.ip()
      ]),
      auth_method: new FormControl(
        this.dataMap.postgre_Auth_Method_Type.database.value,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      database_username: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      database_password: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      children: new FormControl([])
    });

    this.watch();
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

  watch() {
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.Instance_Type.single.value) {
        this.updateSingle();
      } else {
        this.updateCluster();
      }
      this.formGroup.get('agents').updateValueAndValidity();
      this.formGroup.get('port').updateValueAndValidity();
      this.formGroup.get('userName').updateValueAndValidity();
      this.formGroup.get('business_ip').updateValueAndValidity();
      this.formGroup.get('client').updateValueAndValidity();
      this.formGroup
        .get('cluster')
        .updateValueAndValidity({ emitEvent: false });
      this.formGroup.get('children').updateValueAndValidity();
      this.formGroup.get('auth_method').updateValueAndValidity();
      this.formGroup.get('database_password').updateValueAndValidity();
      this.formGroup.get('database_username').updateValueAndValidity();
    });

    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      this.getProxyOptions(res);
      if (!size(this.formGroup.value.children)) {
        return;
      }
      this.formGroup.get('children').setValue([]);
      this.tableData = {
        data: [],
        total: 0
      };
    });
  }

  updateSingle() {
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
      .get('userName')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.maxLength(32)
      ]);
    this.formGroup
      .get('client')
      .setValidators([this.baseUtilService.VALID.required(), this.validPath()]);
    this.formGroup
      .get('business_ip')
      .setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.ip()
      ]);
    this.formGroup
      .get('auth_method')
      .setValidators([this.baseUtilService.VALID.required()]);

    this.formGroup
      .get('database_username')
      .setValidators([this.baseUtilService.VALID.required()]);

    this.formGroup
      .get('database_password')
      .setValidators([this.baseUtilService.VALID.required()]);
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
    this.formGroup.get('port').clearValidators();
    this.formGroup.get('userName').clearValidators();
    this.formGroup.get('client').clearValidators();
    this.formGroup.get('business_ip').clearValidators();
    this.formGroup.get('auth_method').clearValidators();
    this.formGroup.get('database_username').clearValidators();
    this.formGroup.get('database_password').clearValidators();
    setTimeout(() => {
      this.dataTable.fetchData();
    }, 0);
  }

  getProxyOptions(uuid) {
    if (!uuid) {
      return;
    }
    const params = {
      resourceId: uuid
    };
    this.protectedResourceApiService.ShowResource(params).subscribe(res => {
      this.hostNum = res['dependencies']['agents'].length;
    });
  }
  getHostOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.KingBaseInstance.value}Plugin`]
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
        this.hostOptions = hostArray;
      }
    );
  }

  getClusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.KingBaseCluster.value,
        linkStatus: '1'
      })
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
        const clusterArray = [];
        each(recordsTemp, item => {
          clusterArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.clusterOptions = clusterArray;
        return;
      }
      this.getClusterOptions(recordsTemp, startPage);
    });
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'add',
        type: 'primary',
        label: this.i18n.get('common_add_label'),
        disableCheck: () => {
          return !isEmpty(this.item);
        },
        onClick: () => {
          this.add();
        }
      }
    ];

    this.optItems = getPermissionMenuItem(opts);

    const cols: TableCols[] = [
      {
        key: 'hostName',
        name: this.i18n.get('common_host_label'),
        width: 80,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'userName',
        name: this.i18n.get('common_username_label'),
        width: 80,
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
        key: 'databaseUserName',
        name: this.i18n.get('common_database_user_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'operation',
        width: 70,
        useOpWidth: true,
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
                disableCheck: () => {
                  return !isEmpty(this.item);
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
        winTablePagination: true
      }
    };
  }

  add(item?) {
    if (!this.formGroup.value.cluster) {
      this.formGroup.get('cluster').markAsTouched();
      return;
    }

    if (!this.formGroup.get('name')?.value) {
      this.formGroup.get('name').markAsTouched();
      return;
    }
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-host',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader: this.i18n.get('common_add_label'),
        lvContent: KingBaseAddHostComponent,
        lvComponentParams: {
          parentUuid: this.formGroup.value.cluster,
          children: this.formGroup.value.children,
          item,
          portStatus: !isEmpty(this.item),
          hostStatus: !isEmpty(item)
        },
        lvOkDisabled: isEmpty(item) || item?.auth?.authType === 2,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as KingBaseAddHostComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res === 'INVALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as KingBaseAddHostComponent;
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
    return this.formGroup.value.type === DataMap.Instance_Type.single.value
      ? {
          name: this.formGroup.get('name').value,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.KingBaseInstance.value,
          parentUuid: this.formGroup.value.agents,
          extendInfo: {
            hostId: this.formGroup.value.agents,
            instancePort: this.formGroup.get('port').value,
            clientPath: this.formGroup.get('client').value,
            serviceIp: this.formGroup.get('business_ip').value,
            osUsername: this.formGroup.get('userName').value,
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
          subType: DataMap.Resource_Type.KingBaseClusterInstance.value,
          parentUuid: this.formGroup.value.cluster,
          rootUuid: this.formGroup.value.cluster,
          extendInfo: {
            instancePort: this.formGroup.get('port').value,
            isTopInstance: InstanceType.TopInstance
          },
          dependencies: {
            children: this.formGroup.value.children.map(item => {
              const newItem = omit(item, [
                'port',
                'hostName',
                'ip',
                'client',
                'business_ip',
                'parent'
              ]);
              return newItem;
            })
          }
        };
  }

  onOK(): Observable<void> {
    if (
      this.formGroup.value.auth_method ===
      this.dataMap.postgre_Auth_Method_Type.database.value
    ) {
      this.extendsAuth['authType'] = DataMap.Postgre_Auth_Method.db.value;
      this.extendsAuth['authKey'] = this.formGroup.value.database_username;
      this.extendsAuth['authPwd'] = this.formGroup.value.database_password;
    } else {
      this.extendsAuth['authType'] = DataMap.Postgre_Auth_Method.os.value;
      this.extendsAuth['authKey'] = '';
      this.extendsAuth['authPwd'] = '';
    }
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.item) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.item.uuid,
            UpdateResourceRequestBody: params as any
          })
          .subscribe({
            next: () => {
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
