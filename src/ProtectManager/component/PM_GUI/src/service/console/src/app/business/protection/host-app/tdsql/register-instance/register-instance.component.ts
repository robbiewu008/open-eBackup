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
import { MessageboxService } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  MultiCluster,
  getMultiHostOps
} from 'app/shared';
import { TableConfig } from 'app/shared/components/pro-table';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  each,
  filter,
  find,
  get,
  isNumber,
  map,
  first,
  size,
  split,
  last,
  set,
  isEmpty,
  toNumber
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { finalize } from 'rxjs/operators';

@Component({
  selector: 'aui-register-instance',
  templateUrl: './register-instance.component.html',
  styleUrls: ['./register-instance.component.less']
})
export class RegisterInstanceComponent implements OnInit {
  MAX_LENGTH32 = 32;
  MAX_LENGTH64 = 64;
  item;
  dataDetail;
  optsConfig;
  optItems = [];
  proxyOptions = [];
  dataMap = DataMap;
  clusterOptions = [];
  setIdDisabled = false;
  tableData = {
    data: [],
    total: 0
  };
  mysqlVersion;
  dataNodesCache;
  originalDataNodes;
  tableConfig: TableConfig;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  priorityOptions = this.dataMapService
    .toArray('tdsqlNodePriority')
    .map(item => {
      return {
        ...item,
        isLeaf: true
      };
    });
  typeOptions = this.dataMapService.toArray('tdsqlDataNodeType').map(item => {
    return {
      ...item,
      isLeaf: true
    };
  });
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.MAX_LENGTH64
    ])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.MAX_LENGTH32
    ])
  };
  agentsErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_host_number_least_2_label')
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.MAX_LENGTH32
    ])
  };
  setIdErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_setid_errortips_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.MAX_LENGTH64
    ])
  };
  @Input() rowData;
  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    public i18n: I18NService,
    private messageBox: MessageboxService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getClusterOptions();
    this.getProxyOptions();
  }

  initForm() {
    const reg = /^[a-zA-Z_0-9]{0,64}$/;
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(this.MAX_LENGTH64)
        ]
      }),
      cluster: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      setId: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(this.MAX_LENGTH64),
          this.baseUtilService.VALID.name(reg)
        ]
      }),
      username: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(this.MAX_LENGTH32)
        ]
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(this.MAX_LENGTH32)
        ]
      }),
      replPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(this.MAX_LENGTH32)
        ]
      }),
      dataNodes: this.fb.array([], this.baseUtilService.VALID.required())
    });

    if (this.rowData) {
      this.getDataDetail();
    }
  }

  getDataNodesFormGroup(manualAdd = false, rowData?) {
    return this.fb.group({
      manualAdd: new FormControl(manualAdd),
      host: new FormControl(rowData ? rowData.host : ''),
      type: new FormControl(
        rowData
          ? toNumber(rowData.type)
          : DataMap.tdsqlDataNodeType.standby.value
      ),
      defaultFile: new FormControl(rowData ? rowData.defaultFile : '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      socket: new FormControl(rowData ? rowData.socket : '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      proxy: new FormControl(rowData ? rowData.proxy : '', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      priority: new FormControl(
        rowData ? rowData.priority : DataMap.tdsqlNodePriority.medium.value,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      )
    });
  }

  addDataRow(manualAdd = true, rowData?) {
    (this.formGroup.get('dataNodes') as FormArray).push(
      this.getDataNodesFormGroup(manualAdd, rowData)
    );
  }

  deleteDataRow(i) {
    (this.formGroup.get('dataNodes') as FormArray).removeAt(i);
  }

  get dataNodes() {
    return (this.formGroup.get('dataNodes') as FormArray).controls;
  }

  scanDataNodes() {
    if (size(this.dataNodes)) {
      this.repeatScanDataNodes();
    } else {
      this.firstScanDataNodes();
    }
  }

  firstScanDataNodes() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      envId: this.formGroup.value.cluster,
      resourceType: DataMap.Resource_Type.tdsqlCluster.value,
      conditions: JSON.stringify({
        type: DataMap.tdsqlInstanceType.nonDistributed.value,
        id: this.formGroup.get('setId').value
      })
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        const result = first(res.records);
        this.createFormNode(result);
        return;
      });
  }

  private createFormNode(result) {
    const clusterInstanceInfo = JSON.parse(
      get(result, 'extendInfo.clusterInstanceInfo', '{}')
    );
    this.mysqlVersion = get(result, 'extendInfo.mysql_version', null);
    let dataNodes = [];

    each(get(clusterInstanceInfo, 'groups'), group => {
      dataNodes = [
        ...dataNodes,
        ...map(get(group, 'dataNodes'), node => {
          return {
            ...node,
            setId: group.setId
          };
        })
      ];
    });
    this.dataNodesCache = dataNodes;
    this.setIdDisabled = dataNodes.length > 0;
    for (let node of dataNodes) {
      const data = {
        setId: node.setId,
        host: `${node.ip}:${node.port}`,
        type: node.isMaster,
        defaultFile: node.defaultsFile,
        socket: node.socket,
        proxy: '',
        priority: DataMap.tdsqlNodePriority.medium.value
      };

      if (!!this.rowData) {
        const matchNode = find(this.originalDataNodes, item => {
          return item.ip === node.ip;
        });
        if (!!matchNode) {
          set(data, 'proxy', get(matchNode, 'parentUuid', ''));
          set(
            data,
            'priority',
            get(matchNode, 'priority', DataMap.tdsqlNodePriority.medium.value)
          );
        }
      }

      this.addDataRow(false, data);
    }

    if (!!this.rowData) {
      each(this.originalDataNodes, node => {
        if (
          !find(dataNodes, item => {
            return item.ip === node.ip;
          })
        ) {
          const data = {
            host: `${node?.ip}:${node?.port}`,
            type: node?.isMaster,
            defaultFile: node?.defaultsFile,
            socket: node?.socket,
            proxy: node?.parentUuid,
            priority: node?.priority
          };

          this.addDataRow(false, data);
        }
      });
    }

    this.originalDataNodes = [];
  }

  queryDatabaseInfoById() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      envId: this.formGroup.value.cluster,
      resourceType: DataMap.Resource_Type.tdsqlCluster.value,
      conditions: JSON.stringify({
        type: DataMap.tdsqlInstanceType.nonDistributed.value,
        id: this.formGroup.get('setId').value
      })
    };
    return this.protectedEnvironmentApiService.ListEnvironmentResource(params);
  }

  repeatScanDataNodes() {
    this.messageBox.confirm({
      lvDialogIcon: 'lv-icon-popup-danger-48',
      lvContent: this.i18n.get('protection_confirm_scan_data_node_label'),
      lvWidth: MODAL_COMMON.smallWidth + 50,
      lvCancelType: 'default',
      lvOkType: 'primary',
      lvOk: () => {
        while (size(this.dataNodes)) {
          this.deleteDataRow(0);
        }
        this.queryDatabaseInfoById().subscribe(res => {
          const result = first(res.records);
          const clusterInstanceInfo = JSON.parse(
            get(result, 'extendInfo.clusterInstanceInfo', '{}')
          );
          this.mysqlVersion = get(result, 'extendInfo.mysql_version', null);
          let dataNodes = [];

          each(get(clusterInstanceInfo, 'groups'), group => {
            dataNodes = [
              ...dataNodes,
              ...map(get(group, 'dataNodes'), node => {
                return {
                  ...node,
                  setId: group.setId
                };
              })
            ];
          });
          this.dataNodesCache = dataNodes;
          if (!dataNodes.length) {
            this.setIdDisabled = false;
          } else {
            this.setIdDisabled = true;
          }
          for (let node of dataNodes) {
            const data = {
              setId: node.setId,
              host: `${node.ip}:${node.port}`,
              type: node.isMaster,
              defaultFile: node.defaultsFile,
              socket: node.socket,
              proxy: '',
              priority: DataMap.tdsqlNodePriority.medium.value
            };

            this.addDataRow(false, data);
          }
          return;
        });
      }
    });
  }

  getDataDetail() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe(res => {
        const clusterInstanceInfo = JSON.parse(
          get(res, 'extendInfo.clusterInstanceInfo', '{}')
        );
        this.originalDataNodes = get(
          clusterInstanceInfo,
          'groups[0].dataNodes'
        );

        this.formGroup.patchValue({
          type: clusterInstanceInfo.type,
          name: res.name,
          cluster: res.parentUuid,
          setId: clusterInstanceInfo.id,
          username: get(res, 'auth.authKey')
        });
        this.formGroup.get('setId').disable();
        this.createFormNode(this.rowData);
        this.dataDetail = res;
      });
  }

  getClusterOptions(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.tdsqlCluster.value
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
        if (MultiCluster.isMulti && !this.rowData) {
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

  getMysqlVersion(path: string) {
    const MYSQL_START = 'mysql-',
      MARIADB_START = 'mariadb-',
      PERCONA_START = 'percona-';
    if (isEmpty(path)) {
      return this.mysqlVersion;
    }
    return (
      path
        .split('/')
        .find(
          item =>
            item.startsWith(MYSQL_START) ||
            item.startsWith(MARIADB_START) ||
            item.startsWith(PERCONA_START)
        ) || this.mysqlVersion
    );
  }

  getParams() {
    const agents = map(this.formGroup.value.dataNodes, node => {
      return {
        uuid: node.proxy
      };
    });
    const deletedAgents = filter(
      get(this.dataDetail, 'dependencies.agents'),
      item => !find(agents, val => val.uuid === item.uuid)
    );
    const clusterInstanceInfo = {
      name: this.formGroup.get('name').value,
      cluster: this.formGroup.value.cluster,
      id: this.formGroup.get('setId').value,
      type: DataMap.tdsqlInstanceType.nonDistributed.value,
      groups: [
        {
          setId: this.formGroup.get('setId').value,
          dataNodes: map(this.formGroup.value.dataNodes, item => {
            return {
              ip: first(split(item.host, ':')),
              port: last(split(item.host, ':')),
              isMaster: String(item.type),
              defaultsFile: item.defaultFile,
              socket: item.socket,
              parentUuid: item.proxy,
              priority: item.priority,
              linkStatus: get(
                find(this.dataNodesCache, 'ip'),
                'linkStatus',
                '1'
              ),
              nodeType: DataMap.tdsqlNodeType.dataNode.value
            };
          })
        }
      ]
    };

    return {
      name: this.formGroup.get('name').value,
      parentUuid: this.formGroup.get('cluster').value,
      type: 'Database',
      subType: DataMap.Resource_Type.tdsqlInstance.value,
      auth: {
        authType: 2,
        authKey: this.formGroup.value.username,
        authPwd: this.formGroup.value.password,
        extendInfo: {
          replPassword: this.formGroup.get('replPassword').value
        }
      },
      extendInfo: {
        clusterInstanceInfo: JSON.stringify(clusterInstanceInfo),
        mysql_version: this.getMysqlVersion(
          this.formGroup.value.dataNodes[0]?.defaultFile
        )
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
      this.queryDatabaseInfoById()
        .pipe(
          finalize(() => {
            this.createRestoreTask(params, observer);
          })
        )
        .subscribe(res => {
          const result = first(res.records);
          set(
            params,
            'extendInfo.mysql_version',
            get(
              result,
              'extendInfo.mysql_version',
              params.extendInfo.mysql_version
            )
          );
        });
    });
  }

  private createRestoreTask(params, observer: Observer<void>) {
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
    } else {
      this.protectedResourceApiService
        .CreateResource({
          CreateResourceRequestBody: params
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
  }
}
