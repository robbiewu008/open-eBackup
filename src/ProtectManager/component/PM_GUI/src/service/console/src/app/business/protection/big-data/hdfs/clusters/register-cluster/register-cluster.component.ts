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
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService, ModalRef, UploadFile } from '@iux/live';
import { KerberosComponent } from 'app/business/system/security/kerberos/kerberos.component';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  KerberosAPIService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  WarningMessageService,
  ClientManagerApiService,
  MultiCluster,
  Scene,
  Features
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  isEmpty,
  isFunction,
  isNumber,
  isUndefined,
  map,
  omit,
  size,
  toString,
  get,
  each,
  some,
  includes,
  set
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-register-cluster',
  templateUrl: './register-cluster.component.html',
  styleUrls: ['./register-cluster.component.less']
})
export class RegisterClusterComponent implements OnInit {
  data;
  tableData = [];
  kerberosOptions = [];
  proxyHostOptions = [];
  formGroup: FormGroup;
  kerberosTips = '';
  uploadPlaceholder = '';
  isSupport = true;
  authOptions = this.dataMapService
    .toArray('HDFS_Clusters_Auth_Type')
    .map(item => {
      item['isLeaf'] = true;
      return item;
    })
    .filter(item => item.value !== DataMap.HDFS_Clusters_Auth_Type.ldap.value);
  rpcOptions = this.dataMapService
    .toArray('RPC_Protection')
    .filter(item => (item.isLeaf = true));

  clusterAuthType = DataMap.HDFS_Clusters_Auth_Type;
  HDFSNameModeType = DataMap.HDFS_Name_Mode_Type;

  hbaseFilters = [];
  hdfsFilters = [];
  coreFilters = [];
  selectHbaseSiteFile;
  selectHdfsSiteFile;
  selectCoreSiteFile;
  validHbaseSite$ = new Subject<boolean>();
  validHdfsSite$ = new Subject<boolean>();
  validCoreSite$ = new Subject<boolean>();

  nameErrorTip = assign({}, this.baseUtilService.requiredErrorTip, {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  });

  ipErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.ipErrorTip
  );

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private modalRef: ModalRef,
    private message: MessageService,
    private drawModalService: DrawModalService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private kerberosApi: KerberosAPIService,
    private warningMessageService: WarningMessageService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.getAgents();
    this.getKerberos();
    this.initForm();
    this.initFilters();
    this.updateData();
    this.kerberosTips = this.data?.isHbase
      ? this.i18n.get('protection_register_cluster_kerberos_tip_label')
      : this.i18n.get('protection_hdfs_register_cluster_kerberos_tip_label');
    this.uploadPlaceholder = this.data?.uuid
      ? this.i18n.get('protection_big_data_upload_placeholder_label')
      : this.i18n.get('protection_upload_placeholder_label');
  }

  // 判断当前版本是否支持添加存储资源
  isSupportFunc(agent) {
    const params = {
      hostUuidsAndIps: agent,
      applicationType: 'Hbase',
      scene: Scene.Register,
      buttonNames: [Features.SplitTableBackup]
    };
    this.clientManagerApiService
      .queryAgentApplicationUsingPOST({
        AgentCheckSupportParam: params,
        akOperationTips: false
      })
      .subscribe(res => {
        this.isSupport = res?.SplitTableBackup;
      });
  }

  initFilters() {
    this.hbaseFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['xml'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['xml']),
              {
                lvMessageKey: 'formatErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.validHbaseSite$.next(false);
            return validFiles;
          }
          if (files[0].size > 300 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['300KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.validHbaseSite$.next(false);
            return [];
          }

          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectHbaseSiteFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.validHbaseSite$.next(true);
            if (this.data) {
              this.modalRef.getInstance().lvOkDisabled = this.data?.uuid
                ? !(
                    this.formGroup.valid &&
                    isUndefined(
                      this.tableData.find(item => item.isEditing === true)
                    )
                  )
                : !(
                    this.formGroup.valid &&
                    !isEmpty(this.selectHdfsSiteFile) &&
                    !isEmpty(this.selectCoreSiteFile) &&
                    isUndefined(
                      this.tableData.find(item => item.isEditing === true)
                    )
                  );
            }
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];

    this.hdfsFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['xml'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['xml']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            this.validHdfsSite$.next(false);
            return validFiles;
          }
          if (files[0].size > 300 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['300KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );
            this.validHdfsSite$.next(false);
            return [];
          }

          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectHdfsSiteFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.validHdfsSite$.next(true);
            if (this.data) {
              if (this.data?.isHbase) {
                this.modalRef.getInstance().lvOkDisabled = this.data?.uuid
                  ? !(
                      this.formGroup.valid &&
                      isUndefined(
                        this.tableData.find(item => item.isEditing === true)
                      )
                    )
                  : !(
                      this.formGroup.valid &&
                      !isEmpty(this.selectCoreSiteFile) &&
                      !isEmpty(this.selectHbaseSiteFile) &&
                      isUndefined(
                        this.tableData.find(item => item.isEditing === true)
                      )
                    );
              } else {
                this.modalRef.getInstance().lvOkDisabled = this.data?.uuid
                  ? !(
                      this.formGroup.valid &&
                      isUndefined(
                        this.tableData.find(item => item.isEditing === true)
                      )
                    )
                  : !(
                      this.formGroup.valid &&
                      !isEmpty(this.selectCoreSiteFile) &&
                      isUndefined(
                        this.tableData.find(item => item.isEditing === true)
                      )
                    );
              }
            }
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];

    this.coreFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['xml'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['xml']),
              {
                lvMessageKey: 'formatErrorKey2',
                lvShowCloseButton: true
              }
            );
            this.validCoreSite$.next(false);
            return validFiles;
          }
          if (files[0].size > 300 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['300KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey2',
                lvShowCloseButton: true
              }
            );
            this.validCoreSite$.next(false);
            return [];
          }

          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectCoreSiteFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.validCoreSite$.next(true);
            if (this.data) {
              if (this.data?.isHbase) {
                this.modalRef.getInstance().lvOkDisabled = this.data?.uuid
                  ? !(
                      this.formGroup.valid &&
                      isUndefined(
                        this.tableData.find(item => item.isEditing === true)
                      )
                    )
                  : !(
                      this.formGroup.valid &&
                      !isEmpty(this.selectHdfsSiteFile) &&
                      !isEmpty(this.selectHbaseSiteFile) &&
                      isUndefined(
                        this.tableData.find(item => item.isEditing === true)
                      )
                    );
              } else {
                this.modalRef.getInstance().lvOkDisabled = this.data?.uuid
                  ? !(
                      this.formGroup.valid &&
                      isUndefined(
                        this.tableData.find(item => item.isEditing === true)
                      )
                    )
                  : !(
                      this.formGroup.valid &&
                      !isEmpty(this.selectHdfsSiteFile) &&
                      isUndefined(
                        this.tableData.find(item => item.isEditing === true)
                      )
                    );
              }
            }
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  filesChange(file, name) {
    if (!size(file)) {
      switch (name) {
        case 'hbase':
          this.selectHbaseSiteFile = '';
          this.validHbaseSite$.next(false);
          break;
        case 'hdfs':
          this.selectHdfsSiteFile = '';
          this.validHdfsSite$.next(false);
          break;
        case 'core':
          this.selectCoreSiteFile = '';
          this.validCoreSite$.next(false);
          break;
        default:
          break;
      }
    }
  }

  updateData() {
    if (isEmpty(omit(this.data, 'isHbase'))) {
      return;
    }

    if (this.data.auth?.authType === this.clusterAuthType.system.value) {
      assign(this.data, {
        loginMode: this.clusterAuthType.system.value,
        username: this.data.auth?.authKey
      });
    } else {
      assign(this.data, {
        loginMode: this.clusterAuthType.kerberos.value,
        kerberosId: this.data.extendInfo?.kerberosId
      });
    }

    if (!this.data?.isHbase) {
      assign(this.data.extendInfo, {
        isBackupACL: this.data.extendInfo?.isBackupACL
          ? this.data.extendInfo?.isBackupACL === 'true'
          : false
      });
    }

    if (this.data?.isHbase) {
      const { isBackupDivisionTable } = this.data.extendInfo;
      assign(this.data.extendInfo, {
        // 升级上来没有这个字段，则默认为开启
        isBackupDivisionTable: isUndefined(isBackupDivisionTable)
          ? true
          : isBackupDivisionTable === 'true'
      });
    }

    this.formGroup.patchValue({
      ...this.data,
      ...{
        ...this.data.extendInfo,
        agents: this.data.extendInfo?.agents.split(';')
      }
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      loginMode: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      kerberosId: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      username: new FormControl(this.data?.isHbase ? 'hbase' : 'hdfs', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.name)
        ]
      }),
      agents: new FormControl([], {
        validators: [this.baseUtilService.VALID.minLength(1)]
      })
    });

    if (!this.data?.isHbase) {
      this.formGroup.addControl('isBackupACL', new FormControl(false));
    }

    if (this.data?.isHbase) {
      // 是否备份分裂表--HBase
      this.formGroup.addControl('isBackupDivisionTable', new FormControl(true));
    }
    this.listenFormGroup();
  }

  listenFormGroup() {
    this.formGroup.get('loginMode').valueChanges.subscribe(res => {
      if (res === DataMap.HDFS_Clusters_Auth_Type.system.value) {
        this.formGroup.get('kerberosId').clearValidators();
        this.formGroup
          .get('username')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.name)
          ]);
      } else {
        this.formGroup.get('username').clearValidators();
        this.formGroup
          .get('kerberosId')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('username').updateValueAndValidity();
      this.formGroup.get('kerberosId').updateValueAndValidity();
    });

    // hbase主机值变化时，需要调接口查询是否支持分裂表备份
    this.formGroup.get('agents').valueChanges.subscribe(res => {
      if (this.data?.isHbase) {
        if (isEmpty(res)) {
          this.isSupport = true;
        } else {
          this.isSupportFunc(res);
        }
      }
    });
  }

  getKerberos(callback?) {
    this.kerberosApi
      .queryAllKerberosUsingGET({
        startPage: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_OPTIONS[2]
      })
      .subscribe(res => {
        this.kerberosOptions = map(res.items, item => {
          item['isLeaf'] = true;
          item['label'] = item.name;
          return item;
        });
        if (!isUndefined(callback) && isFunction(callback)) {
          callback();
        }
      });
  }

  addData(array: any[], item) {
    array.push({
      ...item,
      key: item.uuid,
      label: `${item.name}(${item.endpoint})`,
      value: item.rootUuid || item.parentUuid,
      isLeaf: true
    });
  }

  // 已选离线主机也回显一下
  isRegisterAgent(uuid) {
    return includes(this.data?.extendInfo?.agents?.split(';'), uuid);
  }

  getAgents(recordsTemp?, startPage?) {
    this.clientManagerApiService
      .queryAgentListInfoUsingGET({
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          pluginType: `${
            this.data?.isHbase
              ? DataMap.Resource_Type.HBaseBackupSet.value
              : DataMap.Resource_Type.HDFSFileset.value
          }Plugin`
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
          startPage ===
            Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE_MAX) ||
          res.totalCount === 0
        ) {
          const agentArr = [];
          if (MultiCluster.isMulti) {
            each(recordsTemp, item => {
              const connection = get(item, 'extendInfo.connection_result');
              if (isEmpty(connection)) {
                if (
                  item.linkStatus ===
                    DataMap.resource_LinkStatus_Special.normal.value ||
                  this.isRegisterAgent(item.rootUuid)
                )
                  this.addData(agentArr, item);
              } else {
                const targetObj = JSON.parse(connection || '{}');
                const linkFlag = some(
                  targetObj,
                  item =>
                    item.link_status ===
                    Number(DataMap.resource_LinkStatus_Special.normal.value)
                );
                //匹配成功 使用对应的状态
                if (linkFlag || this.isRegisterAgent(item.rootUuid)) {
                  this.addData(agentArr, item);
                }
              }
            });
          } else {
            each(recordsTemp, item => {
              if (
                item.linkStatus ===
                  DataMap.resource_LinkStatus_Special.normal.value ||
                this.isRegisterAgent(item.rootUuid)
              ) {
                this.addData(agentArr, item);
              }
            });
          }
          this.proxyHostOptions = agentArr;
          return;
        }
        this.getAgents(recordsTemp, startPage);
      });
  }

  createKerberos() {
    const kerberosComponent = new KerberosComponent(
      this.i18n,
      this.drawModalService
    );
    kerberosComponent.create(undefined, kerberosId => {
      this.getKerberos(() => {
        this.formGroup.get('kerberosId').setValue(kerberosId);
      });
    });
  }

  onOK(): Observable<void> {
    if (this.formGroup.invalid) {
      return;
    }

    const body = {
      name: this.formGroup.value.name,
      type: ResourceType.BIG_DATA,
      subType: this.data?.isHbase
        ? DataMap.Resource_Type.HBase.value
        : DataMap.Resource_Type.HDFS.value,
      extendInfo: {
        kerberosId: this.formGroup.value.kerberosId,
        hdfsSite: this.selectHdfsSiteFile || this.data?.extendInfo?.hdfsSite,
        coreSite: this.selectCoreSiteFile || this.data?.extendInfo?.coreSite,
        hbaseSite: this.selectHbaseSiteFile || this.data?.extendInfo?.hbaseSite,
        agents: toString(this.formGroup.value.agents).replace(/,/g, ';')
      },
      auth: {
        authType: this.formGroup.value.loginMode,
        authKey: this.formGroup.value.username,
        extendInfo: {}
      },
      dependencies: {
        agents: map(this.formGroup.value.agents, item => {
          return {
            uuid: item
          };
        })
      }
    };

    if (this.formGroup.value.loginMode === this.clusterAuthType.system.value) {
      delete body.extendInfo.kerberosId;
    } else if (
      this.formGroup.value.loginMode === this.clusterAuthType.kerberos.value
    ) {
      delete body.auth.authKey;
    }

    if (!this.data?.isHbase) {
      delete body.extendInfo.hbaseSite;
      assign(body.extendInfo, {
        isBackupACL: `${this.formGroup.value.isBackupACL}`
      });
    }

    if (this.data?.isHbase) {
      assign(body.extendInfo, {
        isBackupDivisionTable: this.isSupport
          ? String(this.formGroup.value.isBackupDivisionTable)
          : 'false'
      });
    }
    this.warningMessageService.create({
      content: this.i18n.get('protection_hdfs_hbase_register_warn_label'),
      onOK: () => {
        !isEmpty(omit(this.data, 'isHbase'))
          ? this.onModify(body).subscribe(() => {
              this.modalRef.getInstance().close();
            })
          : this.onCreate(body).subscribe(() => {
              this.modalRef.getInstance().close();
            });
      }
    });
  }

  onModify(body): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.protectedEnvironmentApiService
        .UpdateProtectedEnvironment({
          UpdateProtectedEnvironmentRequestBody: body,
          envId: this.data.uuid
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

  onCreate(body): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.protectedEnvironmentApiService
        .RegisterProtectedEnviroment({
          RegisterProtectedEnviromentRequestBody: body
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
}
