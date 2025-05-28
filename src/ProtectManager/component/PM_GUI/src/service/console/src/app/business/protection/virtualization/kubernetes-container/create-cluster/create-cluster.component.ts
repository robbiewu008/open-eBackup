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
import { Component, OnDestroy, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService, ModalRef, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  first,
  isEmpty,
  isString,
  isUndefined,
  last,
  reject,
  size,
  toNumber,
  uniqueId,
  filter,
  includes,
  intersection,
  map,
  difference,
  some
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-create-cluster',
  templateUrl: './create-cluster.component.html',
  styleUrls: ['./create-cluster.component.less']
})
export class CreateClusterComponent implements OnInit, OnDestroy {
  rowItem: any;
  formGroup: FormGroup;
  dataMap = DataMap;
  isEn = this.i18n.isEn;
  configFileFilter;
  selectConfigFile;
  configFiles = [];
  _isEn = this.i18n.isEn;
  clusterOptions = this.dataMapService
    .toArray('k8sClusterType')
    .filter(item => {
      return (item.isLeaf = true);
    });

  _destory = new Subject();

  includeLabels = [];
  prefixInKey = 'prefixInKey';
  prefixInValue = 'prefixInValue';
  kubeconfigHelp = this.i18n.get('protetion_kubernetes_config_help_label');
  tagHelp = this.i18n.get('protetion_kubernetes_tag_help_label');
  tokenHelp = this.i18n.get('protetion_kubernetes_token_help_label');

  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  tokenErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  taskNumberErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 8])
  };
  podTagErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidPodTag: this.i18n.get('protection_kubernetes_pod_tag_vaild_label')
  };
  keyErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidLabel: this.i18n.get('protection_labels_key_valid_label')
  };
  valueErrorTip = {
    invalidMaxLength: this.i18n.get('protection_labels_value_valid_label'),
    invalidName: this.i18n.get('protection_labels_value_valid_label')
  };
  hostOptions = [];

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private message: MessageService,
    private appUtilsService: AppUtilsService,
    public baseUtilService: BaseUtilService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnDestroy() {
    this._destory.next(true);
    this._destory.complete();
  }

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
    this.initConfigFileFilter();
    this.updateForm();
  }

  configHelpHover() {
    const url1 = this.i18n.isEn
      ? '/console/assets/help/a8000/en-us/index.html#en-us_topic_0000002199956493.html'
      : '/console/assets/help/a8000/zh-cn/index.html#kubernetes_CSI_00013.html';

    const url2 = this.i18n.isEn
      ? '/console/assets/help/a8000/en-us/index.html#en-us_topic_0000002164590142.html'
      : '/console/assets/help/a8000/zh-cn/index.html#kubernetes_CSI_00016.html';
    this.appUtilsService.openSpecialHelp([url1, url2]);
  }

  tagHelpHover() {
    const url = this.i18n.isEn
      ? '/console/assets/help/a8000/en-us/index.html#en-us_topic_0000002199990725.html'
      : '/console/assets/help/a8000/zh-cn/index.html#kubernetes_CSI_00018.html';
    this.appUtilsService.openSpecialHelp(url);
  }

  tokenHelpHover() {
    const url = this.i18n.isEn
      ? '/console/assets/help/a8000/en-us/index.html#en-us_topic_0000002199990881.html'
      : '/console/assets/help/a8000/zh-cn/index.html#kubernetes_CSI_00026.html';
    this.appUtilsService.openSpecialHelp(url);
  }

  getProxyOptions() {
    const extParams = {
      queryDependency: true,
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [
          `${DataMap.Resource_Type.kubernetesClusterCommon.value}Plugin`
        ]
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
        if (MultiCluster.isMulti) {
          resource = filter(resource, item => {
            const val = item.environment;
            const connection = val?.extendInfo?.connection_result;
            const targetObj = JSON.parse(connection || '{}');
            const linkFlag = some(
              targetObj,
              item =>
                item.link_status ===
                Number(DataMap.resource_LinkStatus_Special.normal.value)
            );

            if (
              linkFlag ||
              includes(
                map(this.rowItem?.dependencies?.agents, 'uuid'),
                val.uuid
              )
            ) {
              return true;
            }
            return (
              val.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
            );
          });
        }
        resource = filter(
          resource,
          item =>
            item.environment?.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value ||
            includes(
              map(this.rowItem?.dependencies?.agents, 'uuid'),
              item.environment?.uuid
            )
        );
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
        if (!isEmpty(this.rowItem)) {
          this.formGroup
            .get('clusterNode')
            .setValue(
              intersection(
                map(this.rowItem.dependencies?.agents, 'uuid'),
                map(this.hostOptions, 'uuid')
              ),
              {
                emitEvent: false
              }
            );
        }
      }
    );
  }

  updateForm() {
    if (isEmpty(this.rowItem)) {
      return;
    }
    let taskTimeout;
    let consistentScriptTimeout;
    try {
      taskTimeout = JSON.parse(this.rowItem.extendInfo?.taskTimeout);
      consistentScriptTimeout = JSON.parse(
        this.rowItem.extendInfo?.consistentScriptTimeout
      );
    } catch (error) {
      taskTimeout = {};
      consistentScriptTimeout = {};
    }
    this.formGroup.patchValue({
      type: this.rowItem.auth?.authType,
      name: this.rowItem.name,
      ip: this.rowItem.endpoint,
      port: this.rowItem.port,
      clusterType: this.rowItem.extendInfo?.clusterType,
      podTag: this.rowItem.extendInfo?.imageNameAndTag,
      taskNumber: this.rowItem.extendInfo?.jobNumOnSingleNode,
      cert: this.rowItem.extendInfo?.isVerifySsl === '1',
      timeoutDay: taskTimeout?.days ?? 1,
      timeoutHour: taskTimeout?.hours ?? 0,
      timeoutMin: taskTimeout?.minutes ?? 0,
      timeoutSec: taskTimeout?.seconds ?? 0,
      scriptTimeoutHour: consistentScriptTimeout?.hours ?? 1,
      scriptTimeoutMin: consistentScriptTimeout?.minutes ?? 0,
      scriptTimeoutSec: consistentScriptTimeout?.seconds ?? 0
    });
    if (
      isString(this.rowItem.extendInfo?.nodeSelector) &&
      !isEmpty(this.rowItem.extendInfo?.nodeSelector)
    ) {
      each(this.rowItem.extendInfo?.nodeSelector.split(','), item => {
        this.addIncludeLabels(item.split('=')[0], item.split('=')[1]);
      });
    }
  }

  addIncludeLabels(key?: string, value?: string) {
    const id = uniqueId();
    this.formGroup.addControl(
      this.prefixInKey + id,
      new FormControl(key || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.appUtilsService.validLabel()
        ]
      })
    );
    this.formGroup.addControl(
      this.prefixInValue + id,
      new FormControl(value || '', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.label, false),
          this.baseUtilService.VALID.maxLength(63)
        ]
      })
    );
    this.includeLabels.push({ id });
    this.formGroup.updateValueAndValidity();
  }

  deleteIncludeLabels(id) {
    this.includeLabels = reject(this.includeLabels, v => v.id === id);
    this.formGroup.removeControl(this.prefixInKey + id);
    this.formGroup.removeControl(this.prefixInValue + id);
    this.formGroup.updateValueAndValidity();
  }

  getIncludeLabels() {
    if (isEmpty(this.includeLabels)) {
      return '';
    }
    const arr = [];
    each(this.includeLabels, item => {
      arr.push(
        `${this.formGroup.get(`${this.prefixInKey}${item.id}`)?.value}=${
          this.formGroup.get(`${this.prefixInValue}${item.id}`)?.value
        }`
      );
    });
    return arr.join(',');
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Cluster_Register_Mode.token.value),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      clusterNode: new FormControl([]),
      ip: new FormControl(
        { value: '', disabled: !!this.rowItem?.uuid },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip()
          ]
        }
      ),
      port: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      }),
      clusterType: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      token: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      podTag: new FormControl('', {
        validators: [this.baseUtilService.VALID.required(), this.validPodTag()]
      }),
      taskNumber: new FormControl(4, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 8)
        ]
      }),
      timeoutDay: new FormControl(1),
      timeoutHour: new FormControl(0),
      timeoutMin: new FormControl(0),
      timeoutSec: new FormControl(0),
      scriptTimeoutHour: new FormControl(1),
      scriptTimeoutMin: new FormControl(0),
      scriptTimeoutSec: new FormControl(0),
      cert: new FormControl(true),
      certData: new FormControl('')
    });
    if (isEmpty(this.rowItem)) {
      this.formGroup
        .get('certData')
        .setValidators([this.baseUtilService.VALID.required()]);
    }
    this.listenForm();
  }

  validPodTag(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      if (!control.value) {
        return { required: { value: control.value } };
      }

      const values = control.value.split(':');
      if (values.length < 2) {
        return { invalidPodTag: { value: control.value } };
      }
      const reg = /^[a-zA-Z0-9][a-zA-Z0-9_.-]{0,61}[a-zA-Z0-9]$|^[a-zA-Z0-9]$/;

      if (isEmpty(first(values)) && values.length === 2) {
        return { invalidPodTag: { value: control.value } };
      }

      if (!reg.test(last(values))) {
        return { invalidPodTag: { value: control.value } };
      }
      return null;
    };
  }

  listenForm() {
    this.formGroup
      .get('type')
      .valueChanges.pipe(takeUntil(this._destory))
      .subscribe(res => {
        if (res === DataMap.Cluster_Register_Mode.token.value) {
          this.formGroup
            .get('ip')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.ip()
            ]);
          this.formGroup
            .get('port')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 65535)
            ]);
          this.formGroup
            .get('token')
            .setValidators([this.baseUtilService.VALID.required()]);
          if (this.formGroup.value.cert) {
            this.formGroup
              .get('certData')
              .setValidators([this.baseUtilService.VALID.required()]);
          } else {
            this.formGroup.get('certData').clearValidators();
          }
        } else {
          this.formGroup.get('ip').clearValidators();
          this.formGroup.get('port').clearValidators();
          this.formGroup.get('token').clearValidators();
          this.formGroup.get('certData').clearValidators();
        }
        this.formGroup.get('ip').updateValueAndValidity();
        this.formGroup.get('port').updateValueAndValidity();
        this.formGroup.get('token').updateValueAndValidity();
        this.formGroup.get('certData').updateValueAndValidity();
      });
    this.formGroup
      .get('cert')
      .valueChanges.pipe(takeUntil(this._destory))
      .subscribe(res => {
        if (
          res &&
          DataMap.Cluster_Register_Mode.token.value ===
            this.formGroup.value.type
        ) {
          this.formGroup
            .get('certData')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup.get('certData').clearValidators();
        }
        this.formGroup.get('certData').updateValueAndValidity();
      });
    this.formGroup.statusChanges
      .pipe(takeUntil(this._destory))
      .subscribe(() => this.setOkDisabled());
  }

  initConfigFileFilter() {
    this.configFileFilter = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.selectConfigFile = '';
            this.setOkDisabled();
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectConfigFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.setOkDisabled();
          };
          reader.readAsDataURL(files[0].originFile);
          return files;
        }
      }
    ];
  }

  configFileChange(files) {
    if (size(files) === 0) {
      this.selectConfigFile = '';
      this.setOkDisabled();
    }
  }

  setOkDisabled() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.value.type === DataMap.Cluster_Register_Mode.token.value
        ? this.formGroup.invalid
        : this.formGroup.invalid || isEmpty(this.selectConfigFile);
  }

  getParams() {
    let reduceAgents = [];
    if (!isEmpty(this.rowItem)) {
      reduceAgents = difference(
        this.rowItem.dependencies?.agents.map(item => item.uuid),
        this.formGroup.value.clusterNode
      );
    }
    const params = {
      name: this.formGroup.value.name,
      type: 'KubernetesCommon',
      subType: DataMap.Resource_Type.kubernetesClusterCommon.value,
      dependencies: {
        agents: map(this.formGroup.value.clusterNode, item => {
          return { uuid: item };
        })
      },
      extendInfo: {
        clusterType: this.formGroup.value.clusterType,
        imageNameAndTag: this.formGroup.value.podTag,
        isVerifySsl: this.formGroup.value.cert ? '1' : '0',
        taskTimeout: JSON.stringify({
          days: Number(this.formGroup.value.timeoutDay),
          hours: Number(this.formGroup.value.timeoutHour),
          minutes: Number(this.formGroup.value.timeoutMin),
          seconds: Number(this.formGroup.value.timeoutSec)
        }),
        consistentScriptTimeout: JSON.stringify({
          hours: Number(this.formGroup.value.scriptTimeoutHour),
          minutes: Number(this.formGroup.value.scriptTimeoutMin),
          seconds: Number(this.formGroup.value.scriptTimeoutSec)
        })
      }
    };
    if (!isEmpty(this.rowItem)) {
      assign(params.dependencies, {
        '-agents': reduceAgents.map(item => {
          return { uuid: item };
        })
      });
    }
    if (!isEmpty(this.includeLabels)) {
      assign(params.extendInfo, {
        nodeSelector: this.getIncludeLabels()
      });
    } else {
      assign(params.extendInfo, {
        nodeSelector: ''
      });
    }
    if (this.formGroup.value.taskNumber) {
      assign(params.extendInfo, {
        jobNumOnSingleNode: toNumber(this.formGroup.value.taskNumber)
      });
    }
    if (
      this.formGroup.value.type === DataMap.Cluster_Register_Mode.token.value
    ) {
      assign(params, {
        endpoint: this.formGroup.value.ip || this.rowItem?.endpoint,
        port: toNumber(this.formGroup.value.port),
        auth: {
          authType: DataMap.Cluster_Register_Mode.token.value,
          extendInfo: {
            token: this.formGroup.value.token,
            certificateAuthorityData: this.formGroup.value.cert
              ? this.formGroup.value.certData
              : ''
          }
        }
      });
    } else {
      assign(params, {
        auth: {
          authType: DataMap.Cluster_Register_Mode.kubeconfig.value,
          extendInfo: {
            configKey: this.selectConfigFile
          }
        }
      });
    }
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        observer.error(null);
        observer.complete();
        return;
      }
      const params = this.getParams();
      if (this.rowItem?.uuid) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.rowItem.uuid,
            UpdateProtectedEnvironmentRequestBody: params
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
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params
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
      }
    });
  }
}
