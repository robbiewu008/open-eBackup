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
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { TableConfig } from 'app/shared/components/pro-table';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  each,
  filter,
  find,
  get,
  isEmpty,
  map,
  set,
  size,
  split,
  uniq
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-register',
  templateUrl: './register.component.html',
  styleUrls: ['./register.component.less']
})
export class RegisterComponent implements OnInit {
  item;
  dataDetail;
  optsConfig;
  optItems = [];
  proxyOptions = [];
  rootCertFileFilters = [];
  userCertFilters = [];
  userPrivateKeyFilters = [];
  selectRootCertFile;
  selectUserCert;
  selectUserPrivateKey;
  requireCert = true;
  typeOptions = this.dataMapService.toArray('dbTwoType').map(item => {
    return {
      ...item,
      isLeaf: true
    };
  });
  dataMap = DataMap;

  tableData = {
    data: [],
    total: 0
  };
  tableConfig: TableConfig;
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    pathError: this.i18n.get('common_path_error_label')
  };
  adressErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    samePathError: this.i18n.get('protection_same_path_error_label'),
    invalidName: this.i18n.get('common_invalid_inputtext_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [4096])
  };
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };

  @Input() rowData;
  constructor(
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    public i18n: I18NService,
    private message: MessageService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initFilters();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.name(CommonConsts.REGEX.name)]
      }),
      address: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.vaildIpAdress()
        ]
      }),
      pmPort: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      }),
      iamName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      iamPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      node: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      xbsaConfPath: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validXbsaPath()
        ]
      }),
      xbsa: new FormControl(true),
      privateKeyPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      })
    });

    this.formGroup.statusChanges.subscribe(res => {
      this.enableBtnFn();
    });

    this.formGroup.get('xbsa').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('privateKeyPassword')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(32)
          ]);
      } else {
        this.formGroup.get('privateKeyPassword').clearValidators();
      }
      this.selectRootCertFile = '';
      this.selectUserCert = '';
      this.selectUserPrivateKey = '';
      this.formGroup.get('privateKeyPassword').setValue('');
      this.formGroup.updateValueAndValidity();
    });

    if (this.rowData) {
      this.getDataDetail();
    }
  }

  vaildIpAdress() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }

      if (control.value?.length > 4096) {
        return { invalidMaxLength: { value: control.value } };
      }

      const links = split(control.value, ',')?.filter(item => {
        return !isEmpty(item);
      });
      let invaildInfo;

      if (links.length !== uniq(links).length) {
        return { samePathError: { value: control.value } };
      }

      each(links, link => {
        if (
          !CommonConsts.REGEX.ipv4.test(link) &&
          !CommonConsts.REGEX.ipv6.test(link)
        ) {
          invaildInfo = { invalidName: { value: control.value } };
        }
      });

      return invaildInfo ? invaildInfo : null;
    };
  }

  validXbsaPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) return;
      const reg = /^[^|;&$><`'!+]*$/;
      if (!reg.test(control.value)) {
        return { pathError: { value: control.value } };
      }
      return null;
    };
  }

  initFilters() {
    this.rootCertFileFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          this.valid$.next(false);

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey3',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.selectRootCertFile = '';
            this.enableBtnFn();
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectRootCertFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.enableBtnFn();
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];

    this.userCertFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['crt'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          this.valid$.next(false);

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['crt']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );
            this.selectUserCert = '';
            this.enableBtnFn();
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectUserCert = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.enableBtnFn();
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];

    this.userPrivateKeyFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['key'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          this.valid$.next(false);

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['key']),
              {
                lvMessageKey: 'formatErrorKey2',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey2',
                lvShowCloseButton: true
              }
            );
            this.selectUserPrivateKey = '';
            this.enableBtnFn();
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectUserPrivateKey = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.enableBtnFn();
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
        case 'rootCertFile':
          this.selectRootCertFile = '';
          break;
        case 'userCert':
          this.selectUserCert = '';
          break;
        case 'userPrivateKey':
          this.selectUserPrivateKey = '';
          break;
        default:
          break;
      }
    }
    this.enableBtnFn();
  }

  getDataDetail() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe(res => {
        this.formGroup.patchValue({
          name: res.name,
          address: res.extendInfo?.pmAddress,
          pmPort: res.extendInfo?.pmPort,
          iamName: get(res, 'auth.authKey'),
          node: map(get(res, 'dependencies.agents'), (item: any) => item.uuid),
          xbsaConfPath: res.extendInfo?.xbsaConfPath,
          xbsa: !!get(res, 'extendInfo.caCertPem')
        });

        this.requireCert = !get(res, 'extendInfo.caCertPem');

        this.dataDetail = res;
      });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [
          `${DataMap.Resource_Type.lightCloudGaussdbProject.value}Plugin`
        ]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          }
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  getParams() {
    const deletedNode = filter(
      get(this.dataDetail, 'dependencies.agents'),
      item => !find(this.formGroup.value.node, val => val === item.uuid)
    );
    const caCertPem = this.formGroup.value.xbsa
      ? this.selectRootCertFile
        ? this.selectRootCertFile
        : get(this.rowData, 'extendInfo.caCertPem', '')
      : '';
    const clientCrt = this.formGroup.value.xbsa
      ? this.selectUserCert
        ? this.selectUserCert
        : get(this.rowData, 'extendInfo.clientCrt', '')
      : '';
    const clientKey = this.formGroup.value.xbsa
      ? this.selectUserPrivateKey
        ? this.selectUserPrivateKey
        : get(this.rowData, 'extendInfo.clientKey', '')
      : '';
    const params = {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.lightCloudGaussdbProject.value,
      auth: {
        authType: DataMap.Database_Auth_Method.db.value,
        authKey: this.formGroup.value.iamName,
        authPwd: this.formGroup.value.iamPassword,
        extendInfo: {
          randPass: this.formGroup.value.xbsa
            ? this.formGroup.value.privateKeyPassword
            : ''
        }
      },
      extendInfo: {
        pmAddress: this.formGroup.value.address,
        pmPort: this.formGroup.value.pmPort,
        xbsaConfPath: this.formGroup.value.xbsaConfPath,
        caCertPem: caCertPem,
        clientCrt: clientCrt,
        clientKey: clientKey
      },
      dependencies: {
        agents: map(this.formGroup.value.node, item => {
          return {
            uuid: item
          };
        }),
        '-agents': map(deletedNode, item => {
          return {
            uuid: item.uuid
          };
        })
      }
    };

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

  enableBtnFn() {
    const validFile =
      this.formGroup.value.xbsa && !get(this.rowData, 'extendInfo.caCertPem')
        ? this.selectRootCertFile &&
          this.selectUserCert &&
          this.selectUserPrivateKey
        : true;
    this.valid$.next(validFile);
  }
}
