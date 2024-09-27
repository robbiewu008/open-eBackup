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
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  MessageService,
  ModalRef,
  UploadFile,
  UploadFileStatusEnum
} from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  getMultiHostOps,
  I18NService,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedEnvironmentService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  filter,
  find,
  first,
  isEmpty,
  isUndefined,
  map
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-apsara-stack',
  templateUrl: './register-apsara-stack.component.html',
  styleUrls: ['./register-apsara-stack.component.less']
})
export class RegisterApsaraStackComponent implements OnInit {
  isModify;
  treeSelection;
  resourceType;
  item;
  formGroup: FormGroup;
  proxyOptions = [];
  paramsLength = 1024;
  isSkFocus = false;

  enableCert = false;
  fcCertFilters = [];
  certFiles = [];
  certModifyStatus: boolean = false;
  selectFcSiteFile = '';
  crlFiles = [];
  crlModifyStatus: boolean = false;

  revocationListFilters = [];
  selectRevocationList = '';
  certName = '';
  certSize = '';
  crlName = '';
  crlSize = '';

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip
  };
  endpointErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_ali_endpoint_invalid_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [128]),
    invalidInput: this.i18n.get('common_invalid_input_label')
  };
  maxLengthErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.paramsLength
    ])
  };
  idErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidNameCombination: this.i18n.get('protection_aps_id_tip_label')
  };
  rescanIntervalErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 72])
  };
  urlErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_object_url_invalid_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024])
  };

  constructor(
    public modal: ModalRef,
    public baseUtilService: BaseUtilService,
    public messageService: MessageService,
    private appUtilsService: AppUtilsService,
    private i18n: I18NService,
    private fb: FormBuilder,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private protectedEnvironmentService: ProtectedEnvironmentService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.initFilters();
    this.getProxyOptions();
    if (this.isModify) {
      this.updateData();
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      endpoint: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validEndpoint(),
          this.baseUtilService.VALID.maxLength(128)
        ]
      }),
      ak: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(this.paramsLength)
        ]
      }),
      sk: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(this.paramsLength)
        ]
      }),
      organizationId: new FormControl('', {
        validators: [this.validId(), this.baseUtilService.VALID.required()]
      }),
      regionId: new FormControl('', {
        validators: [this.validId(), this.baseUtilService.VALID.required()]
      }),
      agent: new FormControl([], {
        validators: this.baseUtilService.VALID.required()
      }),
      rescanIntervalInSec: new FormControl(24, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 72)
        ]
      }),
      proxy: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.urlReg, false),
          this.baseUtilService.VALID.maxLength(this.paramsLength)
        ]
      })
    });
    this.formGroup.statusChanges.subscribe(() =>
      defer(() => this.disableOkBtn())
    );
  }

  validId(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || isEmpty(control.value)) {
        return null;
      }

      const reg_domain = /^[a-zA-Z_0-9-]{0,64}$/;
      if (control.value.length > 64) {
        return { invalidNameLength: { value: control.value } };
      }
      if (!reg_domain.test(control.value)) {
        return { invalidNameCombination: { value: control.value } };
      }
      return null;
    };
  }

  validEndpoint(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || isEmpty(control.value)) {
        return null;
      }

      const reg_url = CommonConsts.REGEX.urlReg;
      const reg_ipv4 = CommonConsts.REGEX.ipv4;
      const reg_ipv6 = CommonConsts.REGEX.ipv6;
      if (
        !reg_url.test(control.value) &&
        !reg_ipv4.test(control.value) &&
        !reg_ipv6.test(control.value)
      ) {
        return { invalidName: { value: control.value } };
      }
      return null;
    };
  }

  initFilters() {
    this.fcCertFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.messageService.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            this.selectFcSiteFile = '';
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled = true;
            return '';
          }
          if (files[0].size > 1024 * 1024) {
            this.messageService.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }

          const reader = new FileReader();
          this.certName = first(files)?.name;
          this.certSize = first(files)?.fileSize;
          reader.onloadend = () => {
            this.selectFcSiteFile = atob(
              (reader.result as any).replace('data:', '').replace(/^.+,/, '')
            );
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled =
              !this.selectFcSiteFile || this.formGroup.invalid;
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];

    this.revocationListFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['crl'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.messageService.error(
              this.i18n.get('common_format_error_label', ['crl']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.messageService.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );
            this.selectRevocationList = ''; // 清空
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled = true;
            return '';
          }

          const reader = new FileReader();
          this.crlName = first(files)?.name;
          this.crlSize = first(files)?.fileSize;
          reader.onloadend = () => {
            this.selectRevocationList = atob(
              (reader.result as any).replace('data:', '').replace(/^.+,/, '')
            );
            const modalIns = this.modal.getInstance();
            modalIns.lvOkDisabled =
              (!this.selectFcSiteFile && !this.isModify) ||
              this.formGroup.invalid;
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  updateData() {
    const { name, endpoint, extendInfo } = this.treeSelection[0];
    const { ak, organizationId, regionId, rescanIntervalInSec } = extendInfo;
    this.formGroup.patchValue({
      name,
      endpoint,
      ak,
      rescanIntervalInSec: Number(rescanIntervalInSec) / 3600,
      organizationId,
      regionId,
      sk: '*********',
      agent: extendInfo.agents.split(';')
    });
    if (extendInfo.certName) {
      this.enableCert = true;
      this.certFiles = [
        {
          key: '-1',
          name: extendInfo.certName,
          fileSize: extendInfo.certSize,
          status: UploadFileStatusEnum.SUCCESS
        }
      ];
      this.certName = extendInfo.certName;
      this.certSize = extendInfo.certSize;
    }
    if (extendInfo.crlName) {
      this.crlFiles = [
        {
          key: '-1',
          name: extendInfo.crlName,
          fileSize: extendInfo.crlSize,
          status: UploadFileStatusEnum.SUCCESS
        }
      ];
      this.crlName = extendInfo.crlName;
      this.crlSize = extendInfo.crlSize;
    }
    if (extendInfo.proxy) {
      this.formGroup.get('proxy').setValue(extendInfo.proxy);
    }
    this.formGroup.get('organizationId').disable();
  }

  enableCertChange(e) {
    this.disableOkBtn();
  }

  private _certClear() {
    this.certFiles = [];
    each(['selectFcSiteFile', 'certName', 'certSize'], key => {
      this[key] = '';
    });
    return this;
  }

  private _updateCertModifyStatus() {
    if (this.isModify) {
      this.certModifyStatus = true;
    }
  }

  private _crlClear() {
    this.crlFiles = [];
    each(['selectRevocationList', 'crlName', 'crlSize'], key => {
      this[key] = '';
    });
    return this;
  }

  private _updateCrlModifyStatus() {
    if (this.isModify) {
      this.crlModifyStatus = true;
    }
  }

  cartChange(e) {
    e?.action === 'remove' && this._certClear();
    e?.action === 'ready' && this._updateCertModifyStatus();
    this.disableOkBtn();
  }

  crlChange(e) {
    e?.action === 'remove' && this._crlClear();
    e?.action === 'ready' && this._updateCrlModifyStatus();
    this.disableOkBtn();
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.enableCert
      ? !(
          this.formGroup.status === 'VALID' &&
          (this.certName || this.selectFcSiteFile)
        )
      : this.formGroup.status !== 'VALID';
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`ApsaraStackPlugin`]
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
          resource = getMultiHostOps(resource, true);
        } else {
          resource = filter(resource, val => {
            return (
              val.environment.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
            );
          });
        }
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.endpoint,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  clearSk(e) {
    if (this.isModify && !this.isSkFocus) {
      this.formGroup.get('sk').setValue('');
      this.isSkFocus = true;
    }
  }

  skBlur() {
    if (this.isModify && this.formGroup.value.sk === '') {
      this.formGroup.get('sk').setValue('*********');
      this.isSkFocus = false;
    }
  }

  getParams() {
    let deleteAgents = [];
    if (this.isModify) {
      const oldAgents = this.treeSelection[0].extendInfo?.agents.split(';');
      each(oldAgents, item => {
        if (!find(this.formGroup.value.agent, val => val === item)) {
          deleteAgents.push({
            uuid: item
          });
        }
      });
    }
    const params: any = {
      name: this.formGroup.value.name,
      type: ResourceType.ApsaraStack,
      subType: DataMap.Resource_Type.ApsaraStack.value,
      endpoint: this.isModify
        ? this.treeSelection[0].endpoint
        : this.formGroup.value.endpoint,
      auth: {
        authKey: this.formGroup.value.ak,
        authPwd: this.formGroup.value.sk,
        extendInfo: {
          enableCert: this.enableCert ? '1' : '0'
        }
      },
      extendInfo: {
        agents: this.formGroup.value.agent.join(';'),
        organizationId: this.formGroup.value.organizationId,
        regionId: this.formGroup.value.regionId,
        enableCert: this.enableCert ? '1' : '0',
        rescanIntervalInSec: this.formGroup.value.rescanIntervalInSec * 3600
      },
      dependencies: {
        agents: map(this.formGroup.value.agent, item => {
          return {
            uuid: item
          };
        }),
        '-agents': deleteAgents
      }
    };
    if (this.formGroup.value.sk === '*********') {
      params.auth.authPwd = null;
    }
    if (this.enableCert) {
      assign(params.extendInfo, {
        certName: this.certName,
        certSize: this.certSize
      });
      params.auth.extendInfo.certification = this.selectFcSiteFile || '';
      if (!!this.selectRevocationList) {
        assign(params.extendInfo, {
          crlName: this.crlName,
          crlSize: this.crlSize
        });
        params.auth.extendInfo.revocationList = this.selectRevocationList;
      }
    }
    if (!!this.formGroup.value.proxy) {
      params.extendInfo.proxy = this.formGroup.value.proxy;
    }
    return params;
  }

  onOK(): Observable<void> {
    const params = this.getParams();
    return new Observable<void>((observer: Observer<void>) => {
      if (this.isModify) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            UpdateProtectedEnvironmentRequestBody: this.getParams(),
            envId: this.item.uuid
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
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params as any
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
      }
    });
  }
}
