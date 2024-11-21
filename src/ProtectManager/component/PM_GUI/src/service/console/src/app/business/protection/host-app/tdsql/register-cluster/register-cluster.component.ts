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
import { Component, Input, OnInit } from '@angular/core';
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  getMultiHostOps
} from 'app/shared';
import { TableConfig } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  cloneDeep,
  compact,
  each,
  filter,
  find,
  get,
  isEmpty,
  map,
  uniqBy
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { MultiCluster } from 'app/shared';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
@Component({
  selector: 'aui-register-cluster',
  templateUrl: './register-cluster.component.html',
  styleUrls: ['./register-cluster.component.less']
})
export class RegisterClusterComponent implements OnInit {
  item;
  dataDetail;
  optsConfig;
  optItems = [];
  proxyOptions = [];
  managerProxyOptions = [];
  dataMap = DataMap;
  _find = find;
  repeatNodes = false;
  tableData = {
    data: [],
    total: 0
  };
  tableConfig: TableConfig;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  agentsErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_host_number_least_2_label')
  };
  ipErrorTip = {
    ...this.baseUtilService.ipErrorTip
  };
  portErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  @Input() rowData;
  constructor(
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    public i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      manageNodes: this.fb.array([this.getManageNodesFormGroup()]),
      schedulerNodes: this.fb.array([this.getSchedulerNodesFormGroup()]),
      repeatNodes: new FormControl(0, {
        validators: [this.baseUtilService.VALID.maxSize(0)]
      })
    });
    this.listenForm();
    if (this.rowData) {
      this.getDataDetail();
    }
  }

  listenForm() {
    this.listenSchedulerNodes();
    this.listenMangerNodes();
  }

  listenSchedulerNodes() {
    this.formGroup.get('schedulerNodes').valueChanges.subscribe(res => {
      if (isEmpty(res)) {
        return;
      }
      const selectedProxy = compact(map(res, 'proxy'));
      this.proxyOptions = map(this.proxyOptions, item => {
        item.disabled = selectedProxy.includes(item.value);
        return item;
      });
    });
  }

  listenMangerNodes() {
    this.formGroup.get('manageNodes').valueChanges.subscribe(res => {
      this.repeatNodes = false;
      this.formGroup.get('repeatNodes').setValue(0);
      if (isEmpty(res)) {
        return;
      }
      const map = new Set();
      const filterArr = res.filter(obj =>
        Object.values(obj).every(item => !isEmpty(item))
      );
      for (const item of filterArr) {
        const uniqKey = Object.values(item).join('-');
        if (map.has(uniqKey)) {
          this.repeatNodes = true;
          this.formGroup.get('repeatNodes').setValue(1);
          return;
        }
        map.add(uniqKey);
      }
    });
  }

  getManageNodesFormGroup(data?) {
    return this.fb.group({
      proxy: new FormControl(data ? data.parentUuid : '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      businessIp: new FormControl(data ? data.ip : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID._ipv4()
        ]
      }),
      port: new FormControl(data ? data.port : '8080', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      })
    });
  }

  getSchedulerNodesFormGroup(data?) {
    return this.fb.group({
      proxy: new FormControl(data ? data.parentUuid : '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      businessIp: new FormControl(data ? data.ip : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID._ipv4()
        ]
      })
    });
  }

  addManageRow(data?) {
    (this.formGroup.get('manageNodes') as FormArray).push(
      this.getManageNodesFormGroup(data)
    );
  }

  addSchedulerRow(data?) {
    (this.formGroup.get('schedulerNodes') as FormArray).push(
      this.getSchedulerNodesFormGroup(data)
    );
  }

  deleteManageRow(i) {
    (this.formGroup.get('manageNodes') as FormArray).removeAt(i);
  }

  deleteSchedulerRow(i) {
    (this.formGroup.get('schedulerNodes') as FormArray).removeAt(i);
  }

  get manageNodes() {
    return (this.formGroup.get('manageNodes') as FormArray).controls;
  }

  get schedulerNodes() {
    return (this.formGroup.get('schedulerNodes') as FormArray).controls;
  }

  getDataDetail() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe(res => {
        this.formGroup.patchValue({
          name: res.name,
          username: get(res, 'auth.authKey')
        });

        const clusterInfo = JSON.parse(
          get(res, 'extendInfo.clusterInfo', '{}')
        );
        const ossNodes = get(clusterInfo, 'ossNodes');
        const schedulerNodes = get(clusterInfo, 'schedulerNodes');

        this.deleteManageRow(0);
        this.deleteSchedulerRow(0);

        for (let node of ossNodes) {
          this.addManageRow(node);
        }
        for (let node of schedulerNodes) {
          this.addSchedulerRow(node);
        }
        this.dataDetail = res;
      });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.tdsqlCluster.value}Plugin`]
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
        if (MultiCluster.isMulti && isEmpty(this.rowData)) {
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
        this.proxyOptions = cloneDeep(hostArray);
        this.managerProxyOptions = cloneDeep(hostArray);
      }
    );
  }

  getParams() {
    const agents = uniqBy(
      [
        ...map(this.formGroup.value.manageNodes, node => {
          return {
            uuid: node.proxy
          };
        }),
        ...map(this.formGroup.value.schedulerNodes, node => {
          return {
            uuid: node.proxy
          };
        })
      ],
      'uuid'
    );
    const deletedAgents = filter(
      get(this.dataDetail, 'dependencies.agents'),
      item => !find(agents, val => val.uuid === item.uuid)
    );
    const clusterInfo = {
      ossNodes: map(this.formGroup.value.manageNodes, node => {
        return {
          parentUuid: node.proxy,
          ip: node.businessIp,
          port: node.port,
          nodeType: DataMap.tdsqlNodeType.ossNode.value
        };
      }),
      schedulerNodes: map(this.formGroup.value.schedulerNodes, node => {
        return {
          parentUuid: node.proxy,
          ip: node.businessIp,
          nodeType: DataMap.tdsqlNodeType.schedulerNode.value
        };
      })
    };

    return {
      name: this.formGroup.get('name').value,
      type: 'Database',
      subType: DataMap.Resource_Type.tdsqlCluster.value,
      auth: {
        authType: DataMap.Database_Auth_Method.db.value,
        authKey: this.formGroup.value.username,
        authPwd: this.formGroup.value.password
      },
      extendInfo: {
        clusterInfo: JSON.stringify(clusterInfo)
      },
      dependencies: {
        agents: agents,
        '-agents': deletedAgents
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
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.rowData.uuid,
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
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params
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
