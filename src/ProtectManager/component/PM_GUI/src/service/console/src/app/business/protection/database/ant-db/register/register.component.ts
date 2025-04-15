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
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  InstanceType,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import {
  cacheGuideResource,
  USER_GUIDE_CACHE_DATA
} from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  each,
  find,
  first,
  get,
  includes,
  isEmpty,
  map,
  pick,
  set,
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
  clusterNodesData = []; // lv-datatable使用的默认数据
  dataMap = DataMap;
  formGroup: FormGroup;
  extendsAuth: { [key: string]: any } = {};
  clusterInfo = null; // 缓存集群信息
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
      this.addControlToDataTable(); // 默认填充一个空的集群节点
      this.formGroup.get('nodes').disable(); // 默认是单实例，所以需要禁用集群节点
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
                userName: res.extendInfo.osUsername
              };
        this.formGroup.patchValue(data);
        this.updateTableData(res);
      });
  }

  private updateTableData(res) {
    if (res.subType === DataMap.Resource_Type.AntDBClusterInstance.value) {
      this.clusterInfo = res;
      res.dependencies.children.forEach(item => {
        this.addControlToDataTable(item);
      });
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Instance_Type.single.value),
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.name()]
      }),
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
      nodes: this.fb.array([]) // 集群节点
    });
    this.formGroup
      .get('nodes')
      .setValidators([this.baseUtilService.VALID.minLength(1)]);
    this.clusterNodesData = this.getNodesData();
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
    const controlArr = [
      'agents',
      'business_ip',
      'client',
      'nodes',
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
    this.formGroup.get('nodes').disable();
    this.formGroup.get('databaseStreamUserName').clearValidators();
    this.formGroup.get('databaseStreamPassword').clearValidators();
  }

  updateCluster() {
    this.formGroup.get('nodes').enable();
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

  createNodeControl(item?) {
    return this.fb.group({
      host: new FormControl(
        isEmpty(item) ? '' : item.dependencies.agents[0].uuid,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      client: new FormControl(isEmpty(item) ? '' : item.extendInfo.clientPath, {
        validators: [this.baseUtilService.VALID.required(), this.validPath()]
      }),
      pgPath: new FormControl(
        isEmpty(item) ? '' : item.extendInfo.adbhamgrPath,
        {
          validators: [this.baseUtilService.VALID.required(), this.validPath()]
        }
      ),
      business_ip: new FormControl(
        isEmpty(item) ? '' : item.extendInfo.serviceIp,
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip()
          ]
        }
      ),
      port: new FormControl(
        isEmpty(item) ? '6655' : item.extendInfo.instancePort,
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]
        }
      ),
      nodeValue: new FormControl(isEmpty(item) ? {} : item) // 监视节点，界面上不做展示，只用于缓存之前的数据
    });
  }

  checkValidAndAddControl(item?) {
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
    this.addControlToDataTable(item);
  }

  addControlToDataTable(item?) {
    this.NodesFormArray.push(this.createNodeControl(item));
    this.clusterNodesData = this.getNodesData();
  }

  removeControlFromDataTable(index?: number) {
    this.NodesFormArray.removeAt(index);
    this.clusterNodesData = this.getNodesData();
  }

  get NodesFormArray(): FormArray {
    return this.formGroup.get('nodes') as FormArray;
  }

  getNodesData() {
    return new Array(this.NodesFormArray.length).fill({});
  }

  getClusterNodeParams(nodeControl: AbstractControl) {
    const {
      host: hostId,
      port,
      client,
      pgPath,
      business_ip,
      nodeValue
    } = nodeControl.value;
    const {
      userName,
      databaseStreamUserName,
      databaseStreamPassword
    } = this.formGroup.value;
    const authInfo = {
      ...this.extendsAuth,
      extendInfo: {
        dbStreamRepUser: databaseStreamUserName,
        dbStreamRepPwd: databaseStreamPassword
      }
    };
    const defaultParams = {
      parentUuid: '',
      name: null,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.AntDBInstance.value,
      extendInfo: {
        hostId: hostId,
        instancePort: port,
        clientPath: client,
        adbhamgrPath: pgPath,
        serviceIp: business_ip,
        osUsername: userName,
        isTopInstance: InstanceType.NotTopinstance
      },
      dependencies: {
        agents: [{ uuid: hostId }]
      },
      auth: { ...authInfo }
    };
    if (!isEmpty(nodeValue)) {
      // nodeValue不为空。说明是修改以前已有的节点
      set(nodeValue, 'extendInfo', {
        ...nodeValue.extendInfo,
        ...defaultParams.extendInfo
      });
      set(nodeValue, 'dependencies', defaultParams.dependencies);
      set(nodeValue, 'auth', authInfo);
      set(nodeValue, 'extendInfo.osUsername', this.formGroup.value.userName);
      return nodeValue;
    } else {
      // nodeValue为空。说明是新增新的节点
      return defaultParams;
    }
  }

  getParams() {
    this.extendsAuth.authType = DataMap.Postgre_Auth_Method.db.value;
    this.extendsAuth.authKey = this.formGroup.value.database_username;
    this.extendsAuth.authPwd = this.formGroup.value.database_password;
    let params: any = {
      name: this.formGroup.get('name').value,
      type: ResourceType.DATABASE
    };
    if (
      this.formGroup.get('type').value === DataMap.Instance_Type.single.value
    ) {
      params = {
        ...params,
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
      };
    } else {
      const childNodes: any[] = this.NodesFormArray.controls.map(control =>
        this.getClusterNodeParams(control)
      );
      const oldNodes: any[] = get(
        this.clusterInfo,
        'dependencies.children',
        []
      );
      const newHostIds = new Set(
        childNodes.map(node => node?.extendInfo?.hostId)
      );
      const deprecatedNodes = oldNodes.filter(
        node => !newHostIds.has(node?.extendInfo?.hostId)
      );
      params = {
        ...params,
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
          children: childNodes,
          '-children': deprecatedNodes.map(item => ({
            uuid: item.uuid
          }))
        }
      };
    }
    return params;
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
