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
import { MessageService, ModalRef, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  getMultiHostOps,
  I18NService,
  MultiCluster,
  ProtectedEnvironmentApiService
} from 'app/shared';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  find,
  includes,
  isEmpty,
  isUndefined,
  map,
  size
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-register-object-storage',
  templateUrl: './register-object-storage.component.html',
  styleUrls: ['./register-object-storage.component.less']
})
export class RegisterObjectStorageComponent implements OnInit {
  formGroup: FormGroup;
  protocolOptions = this.dataMapService.toArray('protocolType');
  typeOptions = this.dataMapService.toArray('objectStorageType').map(item => {
    item.isLeaf = true;
    return item;
  });
  proxyOptions = [];
  rowData;
  proxyServer = false;
  isSkFocus = false;
  isPwdFocus = false;
  valid$ = new Subject<boolean>();
  filters = [];
  selectFile;
  certName;
  dataMap = DataMap;
  hostBuiltinLabel = this.i18n.get('protection_hcs_host_builtin_label');
  hostExternalLabel = this.i18n.get('protection_hcs_host_external_label');

  maxLengthErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
  };
  endpointErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInput: this.i18n.get('common_invalid_input_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024])
  };
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip
  };
  serverErrorTip = {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024])
  };
  urlErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_object_url_invalid_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048])
  };
  agentErrorTip = {
    invalidMaxLength: this.i18n.get('common_host_max_number_label', [40])
  };

  constructor(
    public modal: ModalRef,
    public i18n: I18NService,
    public dataMapService: DataMapService,
    public messageService: MessageService,
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private appUtilsService: AppUtilsService,
    private clientManagerApiService: ClientManagerApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.initFilters();
    this.getProxyOptions();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      type: new FormControl('', {
        validators: this.baseUtilService.VALID.required()
      }),
      endpoint: new FormControl(
        { value: '', disabled: !!this.rowData },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.validEndpoint(),
            this.baseUtilService.VALID.maxLength(1024)
          ]
        }
      ),
      protocol: new FormControl('', {
        validators: this.baseUtilService.VALID.required()
      }),
      AK: new FormControl(
        { value: '', disabled: !!this.rowData },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(256)
          ]
        }
      ),
      SK: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256)
        ]
      }),
      agent: new FormControl([], {
        validators: this.baseUtilService.VALID.maxLength(40)
      }),
      serverUrl: new FormControl(''),
      username: new FormControl('', {
        validators: this.baseUtilService.VALID.maxLength(1024)
      }),
      password: new FormControl('', {
        validators: this.baseUtilService.VALID.maxLength(1024)
      })
    });

    this.listenFormGroup();

    if (this.rowData) {
      this.formGroup.patchValue({
        name: this.rowData[0].name,
        type: Number(this.rowData[0].extendInfo.storageType),
        endpoint: this.rowData[0].endpoint,
        protocol: this.rowData[0].extendInfo.useHttps,
        AK: this.rowData[0].extendInfo.ak,
        SK: '*********'
      });
      if (this.rowData[0].extendInfo?.agents) {
        this.formGroup
          .get('agent')
          .setValue(this.rowData[0].extendInfo.agents?.split(';'));
      }
      if (this.rowData[0].extendInfo.proxyEnable === '1') {
        this.proxyServer = true;
        this.serverChange();
        this.formGroup
          .get('serverUrl')
          .setValue(this.rowData[0].extendInfo.proxyHostName);
        if (this.rowData[0].extendInfo.proxyUserName) {
          this.formGroup
            .get('username')
            .setValue(this.rowData[0].extendInfo.proxyUserName);
          this.formGroup.get('password').setValue('********');
        }
      }
    }
  }

  validEndpoint(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || isEmpty(control.value)) {
        return null;
      }

      const value = control.value.split(':');
      const domain_ip_value = value[0];
      let port;
      if (value.length > 1) {
        port = value[1];
        if (!port || isNaN(port) || port < 0 || port > 65535) {
          return { invalidInput: { value: control.value } };
        }
      }

      const reg_domain = /^((?:(?!-)[a-zA-Z0-9-]{1,63}(?<!-)\.)+[a-zA-Z]{2,63})$/;
      const reg_ipv4 = CommonConsts.REGEX.ipv4;

      if (
        !reg_domain.test(domain_ip_value) &&
        !reg_ipv4.test(domain_ip_value)
      ) {
        return { invalidInput: { value: control.value } };
      }
      return null;
    };
  }

  listenFormGroup() {
    this.formGroup.statusChanges.subscribe(res => {
      this.disableBtn();
    });
  }

  disableBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.status !== 'VALID';
  }

  initFilters() {
    this.filters = [
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
                lvMessageKey: 'formatErrorKey3',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.messageService.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.selectFile = '';
            return '';
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.certName = validFiles[0].name;
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
        case 'pem':
          this.selectFile = '';
          break;
        default:
          break;
      }
    }
  }

  serverChange(e?) {
    if (!this.proxyServer) {
      this.formGroup.get('serverUrl').disable();
    } else {
      this.formGroup.get('serverUrl').enable();
      this.formGroup
        .get('serverUrl')
        .setValidators([
          this.baseUtilService.VALID.name(CommonConsts.REGEX.urlHttpReg),
          this.baseUtilService.VALID.maxLength(2048)
        ]);
      this.formGroup.get('serverUrl').updateValueAndValidity();
    }
  }

  clearPwd(e) {
    if (this.rowData && !this.isSkFocus) {
      this.formGroup.get('password').setValue('');
      this.isPwdFocus = true;
    }
  }

  pwdBlur() {
    if (this.rowData && this.formGroup.value.password === '') {
      this.formGroup.get('password').setValue('********');
      this.isPwdFocus = false;
    }
  }

  clearSk(e) {
    if (this.rowData && !this.isSkFocus) {
      this.formGroup.get('SK').setValue('');
      this.isSkFocus = true;
    }
  }

  skBlur() {
    if (this.rowData && this.formGroup.value.SK === '') {
      this.formGroup.get('SK').setValue('*********');
      this.isSkFocus = false;
    }
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        pluginType: `ObjectStoragePlugin`
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        const hostArray = [];
        if (MultiCluster.isMulti) {
          resource = getMultiHostOps(resource, true);
        }
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.endpoint,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true
          });
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  getParams() {
    let deleteAgents = [];
    if (this.rowData && !!this.rowData[0].extendInfo?.agents) {
      const oldAgents = this.rowData[0].extendInfo?.agents.split(';');
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
      type: 'ObjectStorage',
      subType: DataMap.Resource_Type.ObjectStorage.value,
      endpoint: this.formGroup.get('endpoint').value,
      extendInfo: {
        storageType: this.formGroup.value.type,
        agents: ''
      },
      auth: {
        extendInfo: {
          ak: this.formGroup.get('AK').value,
          sk: this.formGroup.value.SK,
          useHttps: this.formGroup.value.protocol,
          proxyEnable: this.proxyServer ? '1' : '0'
        }
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
    if (this.formGroup.value.SK === '*********') {
      params.auth.extendInfo.sk = null;
    }
    if (!!this.formGroup.value?.agent.length) {
      params.extendInfo.agents = this.formGroup.value.agent.join(';');
    }
    if (
      !!this.selectFile &&
      this.formGroup.value.protocol === '1' &&
      this.formGroup.value.type === DataMap.objectStorageType.pacific.value
    ) {
      params.auth.extendInfo.certification = this.selectFile;
      params.auth.extendInfo.certName = this.certName;
      params.extendInfo.certName = this.certName;
    }
    if (this.proxyServer) {
      const password =
        this.formGroup.value.password === '********'
          ? null
          : this.formGroup.value.password;
      assign(params.auth.extendInfo, {
        proxyHostName: this.formGroup.value.serverUrl,
        proxyUserName: this.formGroup.value.username,
        proxyUserPwd: password
      });
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
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.rowData[0].uuid,
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
