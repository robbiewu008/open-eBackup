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
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService, ModalRef } from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  MultiCluster,
  MODAL_COMMON,
  CommonConsts,
  getMultiHostOps
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import {
  assign,
  cloneDeep,
  differenceBy,
  each,
  find,
  includes,
  map,
  size,
  filter,
  isEmpty
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { BatchConfigPathComponent } from './batch-config-path/batch-config-path.component';

@Component({
  selector: 'aui-register-mongodb',
  templateUrl: './register-mongodb.component.html',
  styleUrls: ['./register-mongodb.component.less']
})
export class RegisterMongodbComponent implements OnInit {
  rowItem: any;
  formGroup: FormGroup;
  dataMap = DataMap;
  _find = find;
  proxyOptions: any = [];
  nodeProxyOptions = [];
  clusterNodes: any = [];
  USER_MAX_LENGTH = 32;
  MAX_PATH_LENGTH = 255;
  authOptions = this.dataMapService
    .toArray('Postgre_Auth_Method')
    .filter(item => {
      return (item.isLeaf = true);
    });
  clusterTypeOptions = this.dataMapService
    .toArray('mongodbClusterType')
    .filter(item => {
      item.isLeaf = true;
      return !includes([DataMap.mongodbClusterType.primary.value], item.value);
    });

  originalNodes = [];
  configPathEnum = {
    dbPath: '1',
    dbToolPath: '2'
  };

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  dbErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.USER_MAX_LENGTH
    ])
  };
  pathErrorTip = {
    invalidName: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.MAX_PATH_LENGTH
    ])
  };

  constructor(
    private modal: ModalRef,
    private fb: FormBuilder,
    private i18n: I18NService,
    private messageService: MessageService,
    private infoMessageService: InfoMessageService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getHosts();
    this.updateForm();
    this.getResource();
  }

  getResource() {
    if (!this.rowItem || !this.rowItem.uuid) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowItem.uuid })
      .subscribe((res: any) => {
        if (
          this.rowItem.subType ===
          DataMap.Resource_Type.MongodbSingleInstance.value
        ) {
          this.formGroup
            .get('agent')
            .patchValue(res.dependencies?.agents[0]?.uuid);
        } else {
          const nodes = map(res.dependencies?.children, item => {
            return {
              uuid: item.uuid,
              agentUuid: item.extendInfo?.agentUuid,
              host: item.name,
              ip: item.extendInfo?.serviceIp,
              port: item.extendInfo?.servicePort,
              authType: item.auth?.authType,
              authKey: item.auth?.authKey,
              authPwd: ''
            };
          });
          (this.formGroup.get('nodesConfig') as FormArray).clear();
          each(res.dependencies?.children, item => {
            const params = {
              nodeHost: item.extendInfo?.agentUuid,
              nodePort: item.extendInfo?.servicePort,
              nodeAuth: item.auth?.authType,
              nodeDatabaseUsername: item.auth?.authKey,
              nodeDatabasePassword: '',
              nodeBinPath: item.extendInfo?.binPath,
              nodeMongodumpBinPath: item.extendInfo?.mongodumpBinPath
            };
            const form = this.getNodeFormGroup(true);
            form.patchValue(params);
            (this.formGroup.get('nodesConfig') as FormArray).controls.push(
              form
            );
            form.get('nodePort').disable();
          });
          this.originalNodes = cloneDeep(nodes);
        }
        this.disableOkBtn();
      });
  }

  updateForm() {
    if (!this.rowItem) {
      return;
    }
    const params = {
      type: this.rowItem.subType,
      name: this.rowItem.name
    };
    if (
      this.rowItem.subType === DataMap.Resource_Type.MongodbSingleInstance.value
    ) {
      assign(params, {
        port: this.rowItem.extendInfo?.servicePort,
        authMethod: this.rowItem.auth?.authType,
        databaseUsername: this.rowItem.auth?.authKey,
        binPath: this.rowItem.extendInfo?.binPath || '',
        mongodumpBinPath: this.rowItem.extendInfo?.mongodumpBinPath || '',
        logBackup:
          this.rowItem.extendInfo?.singleType ===
          DataMap.mongoDBSingleInstanceType.copySet.value
      });
    } else {
      assign(params, {
        clusterType: this.rowItem.extendInfo?.clusterType
      });
    }
    this.formGroup.patchValue(params);
    this.disableOkBtn();
  }

  getNodeFormGroup(needValid?) {
    return this.fb.group({
      nodeHost: new FormControl('', {
        validators: needValid ? [this.baseUtilService.VALID.required()] : null
      }),
      nodePort: new FormControl('', {
        validators: needValid
          ? [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 65535)
            ]
          : null
      }),
      nodeAuth: new FormControl('', {
        validators: needValid ? [this.baseUtilService.VALID.required()] : null
      }),
      nodeDatabaseUsername: new FormControl(''),
      nodeDatabasePassword: new FormControl(''),
      nodeBinPath: new FormControl(''),
      nodeMongodumpBinPath: new FormControl('')
    });
  }

  listenNode() {
    each((this.formGroup.get('nodesConfig') as FormArray).controls, form => {
      form.get('nodeAuth').valueChanges.subscribe(res => {
        if (res === DataMap.Postgre_Auth_Method.db.value) {
          form
            .get('nodeDatabaseUsername')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(this.USER_MAX_LENGTH)
            ]);
          form
            .get('nodeDatabasePassword')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(this.USER_MAX_LENGTH)
            ]);
        } else {
          form.get('nodeDatabaseUsername').clearValidators();
          form.get('nodeDatabasePassword').clearValidators();
        }
        form.get('nodeDatabaseUsername').updateValueAndValidity();
        form.get('nodeDatabasePassword').updateValueAndValidity();
      });
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Resource_Type.MongodbSingleInstance.value),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      agent: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      port: new FormControl(
        { value: '', disabled: !!this.rowItem },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]
        }
      ),
      authMethod: new FormControl(DataMap.Postgre_Auth_Method.os.value),
      databaseUsername: new FormControl(''),
      databasePassword: new FormControl(''),
      clusterType: new FormControl(''),
      binPath: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(this.MAX_PATH_LENGTH),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath, false)
        ]
      }),
      mongodumpBinPath: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(this.MAX_PATH_LENGTH),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath, false)
        ]
      }),
      nodesConfig: this.fb.array(
        this.rowItem?.uuid ? [] : [this.getNodeFormGroup()]
      ),
      logBackup: new FormControl(false)
    });
    this.listenForm();
    this.listenNode();
    this.formGroup.statusChanges.subscribe(() => this.disableOkBtn());
  }

  get nodesConfig() {
    return (this.formGroup.get('nodesConfig') as FormArray).controls;
  }

  addNode() {
    (this.formGroup.get('nodesConfig') as FormArray).push(
      this.getNodeFormGroup(true)
    );
    this.listenNode();
  }

  deleteNode(i) {
    (this.formGroup.get('nodesConfig') as FormArray).removeAt(i);
    this.disableOkBtn();
  }

  switchLogBackupStatus() {
    const logValue = !this.formGroup.value.logBackup;
    if (!!this.rowItem) {
      this.infoMessageService.create({
        content: this.i18n.get(
          'protection_mongo_register_log_backup_tips_label'
        ),
        onOK: () => {
          this.formGroup.get('logBackup').setValue(logValue);
        }
      });
    } else {
      this.formGroup.get('logBackup').setValue(logValue);
    }
  }

  listenForm() {
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === DataMap.Resource_Type.MongodbSingleInstance.value) {
        this.formGroup
          .get('agent')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('port')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]);
        each(['binPath', 'mongodumpBinPath'], key => {
          this.formGroup
            .get(key)
            ?.setValidators([
              this.baseUtilService.VALID.maxLength(this.MAX_PATH_LENGTH),
              this.baseUtilService.VALID.name(
                CommonConsts.REGEX.linuxPath,
                false
              )
            ]);
        });
        this.formGroup.get('clusterType').clearValidators();
        each(
          (this.formGroup.get('nodesConfig') as FormArray).controls,
          form => {
            each(
              [
                'nodeHost',
                'nodePort',
                'nodeAuth',
                'nodeDatabaseUsername',
                'nodeDatabasePassword',
                'nodeBinPath',
                'nodeMongodumpBinPath'
              ],
              key => {
                form.get(key)?.clearValidators();
                form.get(key)?.updateValueAndValidity();
              }
            );
          }
        );
        if (
          this.formGroup.value.authMethod ===
          DataMap.Postgre_Auth_Method.db.value
        ) {
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
      } else {
        this.formGroup.get('agent').clearValidators();
        this.formGroup.get('port').clearValidators();
        this.formGroup.get('databaseUsername').clearValidators();
        this.formGroup.get('databasePassword').clearValidators();
        this.formGroup.get('binPath').clearValidators();
        this.formGroup.get('mongodumpBinPath').clearValidators();
        this.formGroup
          .get('clusterType')
          .setValidators([this.baseUtilService.VALID.required()]);
        each(
          (this.formGroup.get('nodesConfig') as FormArray).controls,
          form => {
            form
              .get('nodeHost')
              .setValidators([this.baseUtilService.VALID.required()]);
            form
              .get('nodePort')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.integer(),
                this.baseUtilService.VALID.rangeValue(1, 65535)
              ]);
            form
              .get('nodeAuth')
              .setValidators([this.baseUtilService.VALID.required()]);
            if (
              form.get('nodeAuth').value ===
              DataMap.Postgre_Auth_Method.db.value
            ) {
              form
                .get('nodeDatabaseUsername')
                .setValidators([this.baseUtilService.VALID.required()]);
              form
                .get('nodeDatabasePassword')
                .setValidators([this.baseUtilService.VALID.required()]);
            } else {
              form.get('nodeDatabaseUsername').clearValidators();
              form.get('nodeDatabasePassword').clearValidators();
            }
            form
              .get('nodeBinPath')
              .setValidators([
                this.baseUtilService.VALID.maxLength(this.MAX_PATH_LENGTH),
                this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.linuxPath,
                  false
                )
              ]);
            form
              .get('nodeMongodumpBinPath')
              .setValidators([
                this.baseUtilService.VALID.maxLength(this.MAX_PATH_LENGTH),
                this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.linuxPath,
                  false
                )
              ]);
            form.get('nodeHost').updateValueAndValidity();
            form.get('nodePort').updateValueAndValidity();
            form.get('nodeAuth').updateValueAndValidity();
            form.get('nodeDatabaseUsername').updateValueAndValidity();
            form.get('nodeDatabasePassword').updateValueAndValidity();
            form.get('nodeBinPath').updateValueAndValidity();
            form.get('nodeMongodumpBinPath').updateValueAndValidity();
          }
        );
      }
      this.formGroup.get('clusterType').updateValueAndValidity();
      this.formGroup.get('agent').updateValueAndValidity();
      this.formGroup.get('port').updateValueAndValidity();
      this.formGroup.get('databaseUsername').updateValueAndValidity();
      this.formGroup.get('databasePassword').updateValueAndValidity();
      this.formGroup.get('binPath').updateValueAndValidity();
      this.formGroup.get('mongodumpBinPath').updateValueAndValidity();
    });
    this.formGroup.get('authMethod').valueChanges.subscribe(res => {
      if (res === DataMap.Postgre_Auth_Method.db.value) {
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
  }

  getHosts() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.MongodbSingleInstance.value}Plugin`]
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
        if (!this.rowItem) {
          if (!MultiCluster.isMulti) {
            resource = filter(
              resource,
              item =>
                item.environment.linkStatus ===
                DataMap.resource_LinkStatus_Special.normal.value
            );
          } else {
            resource = getMultiHostOps(resource);
          }
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
        this.nodeProxyOptions = hostArray;
      }
    );
  }

  batchPath(type) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'batch-path-config',
        lvWidth: MODAL_COMMON.normalWidth,
        lvHeader:
          type === this.configPathEnum.dbPath
            ? this.i18n.get('protection_batch_config_db_install_path_label')
            : this.i18n.get(
                'protection_batch_config_db_tool_install_path_label'
              ),
        lvContent: BatchConfigPathComponent,
        lvComponentParams: {
          type,
          configPathEnum: this.configPathEnum
        },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as BatchConfigPathComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(status => {
            modalIns.lvOkDisabled = status !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          const content = modal.getContentComponent() as BatchConfigPathComponent;
          each(
            (this.formGroup.get('nodesConfig') as FormArray).controls,
            form => {
              if (type === this.configPathEnum.dbPath) {
                form.get('nodeBinPath').setValue(content.formGroup?.value.path);
              } else {
                form
                  .get('nodeMongodumpBinPath')
                  .setValue(content.formGroup?.value.path);
              }
            }
          );
        }
      })
    );
  }

  getParams() {
    if (
      this.formGroup.value.type ===
      DataMap.Resource_Type.MongodbSingleInstance.value
    ) {
      const host = find(this.proxyOptions, {
        value: this.formGroup.value.agent
      });
      return {
        name: this.formGroup.value.name,
        type: ResourceType.DATABASE,
        subType: this.formGroup.value.type,
        auth: {
          authType: this.formGroup.value.authMethod,
          authKey:
            this.formGroup.value.authMethod ===
            DataMap.Postgre_Auth_Method.db.value
              ? this.formGroup.value.databaseUsername
              : '',
          authPwd:
            this.formGroup.value.authMethod ===
            DataMap.Postgre_Auth_Method.db.value
              ? this.formGroup.value.databasePassword
              : ''
        },
        extendInfo: {
          serviceIp: host?.extendInfo?.subNetFixedIp || host?.endpoint,
          servicePort: this.formGroup.get('port').value,
          isTopInstance: '1',
          agentUuid: this.formGroup.value.agent,
          binPath: this.formGroup.value.binPath,
          mongodumpBinPath: this.formGroup.value.mongodumpBinPath,
          singleType: this.formGroup.value.logBackup
            ? DataMap.mongoDBSingleInstanceType.copySet.value
            : DataMap.mongoDBSingleInstanceType.single.value
        },
        dependencies: {
          agents: [
            {
              uuid: this.formGroup.value.agent
            }
          ]
        }
      };
    }
    const children = map(
      (this.formGroup.get('nodesConfig') as FormArray).controls,
      form => {
        const item = form.value;
        const host = find(this.nodeProxyOptions, {
          value: item.nodeHost
        });
        const params = {
          name: host?.name,
          type: ResourceType.DATABASE,
          subType: DataMap.Resource_Type.MongodbSingleInstance.value,
          parentUuid: item.nodeHost,
          auth: {
            authType: item.nodeAuth,
            authKey: item.nodeDatabaseUsername,
            authPwd: item.nodeDatabasePassword
          },
          extendInfo: {
            serviceIp: host?.extendInfo?.subNetFixedIp || host?.endpoint,
            servicePort: item.nodePort || form.get('nodePort').value,
            isTopInstance: '0',
            agentUuid: item.nodeHost,
            binPath: item.nodeBinPath,
            mongodumpBinPath: item.nodeMongodumpBinPath
          },
          dependencies: {
            agents: [
              {
                uuid: item.nodeHost
              }
            ]
          }
        };
        if (this.rowItem && !isEmpty(this.originalNodes)) {
          assign(params, {
            uuid: find(
              this.originalNodes,
              node =>
                node.ip === params.extendInfo?.serviceIp &&
                node.port === params.extendInfo?.servicePort
            )?.uuid
          });
        }
        return params;
      }
    );
    const params = {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: this.formGroup.value.type,
      extendInfo: {
        clusterType: this.formGroup.value.clusterType,
        isTopInstance: '1'
      },
      dependencies: {
        children
      }
    };
    if (this.rowItem) {
      const removeChildren = differenceBy(
        this.originalNodes,
        map((this.formGroup.get('nodesConfig') as FormArray).controls, form => {
          const n = form.value;
          const host = find(this.nodeProxyOptions, {
            value: n.nodeHost
          });
          return {
            ip: host?.extendInfo?.subNetFixedIp || host?.endpoint,
            port: n.nodePort || form.get('nodePort').value
          };
        }),
        item => {
          return `${item.ip}${item.port}`;
        }
      );
      assign(params.dependencies, {
        '-children': removeChildren
      });
    }
    return params;
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.value.type ===
      DataMap.Resource_Type.MongodbSingleInstance.value
        ? this.formGroup.invalid
        : this.formGroup.invalid ||
          size((this.formGroup.get('nodesConfig') as FormArray).controls) < 2;
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const params = this.getParams();
      if (this.rowItem) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.rowItem.uuid,
            UpdateProtectedEnvironmentRequestBody: params
          })
          .subscribe(
            res => {
              observer.next(res);
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params
          })
          .subscribe(
            res => {
              observer.next(res);
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      }
    });
  }
}
