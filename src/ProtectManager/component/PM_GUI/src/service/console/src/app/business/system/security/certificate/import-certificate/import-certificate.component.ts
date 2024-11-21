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
  FormBuilder,
  FormControl,
  FormGroup,
  Validators
} from '@angular/forms';
import { MessageService, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  MultiCluster,
  WarningMessageService
} from 'app/shared';
import { ComponentRestApiService } from 'app/shared/api/services';
import { assign, isFunction, toString, find, without, includes } from 'lodash';
import { combineLatest, Subject } from 'rxjs';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-import-certificate',
  templateUrl: './import-certificate.component.html',
  styles: [
    `
      .lv-fileupload {
        width: 100%;
      }
    `
  ]
})
export class ImportCertificateComponent implements OnInit {
  MultiCluster = MultiCluster;
  formGroup: FormGroup;
  validCertificate$ = new Subject<boolean>();
  valid$ = new Subject<boolean>();
  validAgentKey$ = new Subject<boolean>();
  validAgentCertificate$ = new Subject<boolean>();
  validDhparam$ = new Subject<boolean>();
  privateKey$ = new Subject<boolean>();
  internalFlag;
  filters1 = [];
  filters2 = [];
  filters3 = [];
  clientFilters = [];
  privateKeyFilters = [];
  dhparamFilters = [];
  currentComponent;
  selectCertificateFile;
  selectCaCertificateFile;
  selectPrivateKeyFile;
  selectAgentKey;
  selectAgentCertificate;
  selectDhparamCertificate;
  certificateFileLabel = this.i18n.get('system_client_certificate_label');
  caCertificateFileLabel = this.i18n.get('system_ca_certificate_label');
  privateKeyFileLabel = this.i18n.get(
    'system_certificate_server_private_key_label'
  );
  requiredLabel = this.i18n.get('common_required_label');
  passwordLabel = this.i18n.get(
    'system_certificate_server_private_password_label'
  );

  requiredErrorTip = {
    required: this.requiredLabel
  };
  passwordErrorTip = {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [512]),
    required: this.requiredLabel,
    invalidName: this.i18n.get('system_cert_valid_password_label')
  };
  communicationComponentFlag;
  redisComponentFlag;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  isProtectAgent = false;
  isADFS;
  lvAcceptType;

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public certApiService: ComponentRestApiService,
    public dataMapService: DataMapService,
    public warningMessageService: WarningMessageService,
    public message: MessageService,
    public baseUtilService: BaseUtilService,
    public cookieService: CookieService
  ) {}

  initForm() {
    this.internalFlag =
      this.currentComponent.type === DataMap.Component_Type.internal.value ||
      this.currentComponent.type === DataMap.Component_Type.protectAgent.value;
    this.isProtectAgent =
      this.currentComponent.type === DataMap.Component_Type.protectAgent.value;
    this.communicationComponentFlag =
      this.currentComponent.type ===
      DataMap.Component_Type.communicationComponent.value;
    this.redisComponentFlag =
      this.currentComponent.type ===
      DataMap.Component_Type.redisComponent.value;
    this.isADFS =
      this.currentComponent.type === DataMap.Component_Type.adfs.value;
    this.lvAcceptType = this.isADFS ? '.pem,.cer' : '.pem';
    this.formGroup = this.fb.group({
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(512)]
      }),
      agentPassword: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(512)]
      })
    });

    this.filters1 = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = this.isADFS ? ['pem', 'cer'] : ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            const errorMassageArr = [
              'pem',
              this.i18n.get('common_or_label'),
              'cer'
            ];
            this.message.error(
              this.isADFS
                ? this.i18n.get('common_format_error_label', [
                    `${errorMassageArr.join(this.i18n.isEn ? ' ' : '')}`
                  ])
                : this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.validCertificate$.next(false);
            return;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.validCertificate$.next(false);
            return;
          }
          this.validCertificate$.next(true);
          return validFiles;
        }
      }
    ];
    this.filters3 = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });
          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.valid$.next(false);
            return;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.valid$.next(false);
            return;
          }
          this.valid$.next(true);
          return validFiles;
        }
      }
    ];

    this.filters2 = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.privateKey$.next(false);
            return;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.privateKey$.next(false);
            return;
          }
          this.privateKey$.next(true);
          this.selectPrivateKeyFile = files[0].originFile;
          if (this.internalFlag) {
            this.formGroup
              .get('password')
              .setValidators([
                Validators.required,
                this.baseUtilService.VALID.maxLength(512)
              ]);
            this.formGroup.get('password').updateValueAndValidity();
          }
          if (this.communicationComponentFlag) {
            this.formGroup
              .get('password')
              .setValidators([
                Validators.required,
                this.baseUtilService.VALID.name(
                  CommonConsts.REGEX.communicationComponent
                )
              ]);
            this.formGroup.get('password').updateValueAndValidity();
          }
          return validFiles;
        }
      }
    ];

    this.clientFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.validAgentCertificate$.next(false);
            return;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.validAgentCertificate$.next(false);
            return;
          }
          this.validAgentCertificate$.next(true);
          return validFiles;
        }
      }
    ];

    this.privateKeyFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.validAgentKey$.next(false);
            return;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.validAgentKey$.next(false);
            return;
          }
          this.validAgentKey$.next(true);
          return validFiles;
        }
      }
    ];

    this.dhparamFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.validDhparam$.next(false);
            return;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.validDhparam$.next(false);
            return;
          }
          this.validDhparam$.next(true);
          return validFiles;
        }
      }
    ];
  }

  importCertificate(cb, resolve) {
    const params = {
      componentId: this.currentComponent.componentId,
      caCertificate: this.selectCaCertificateFile,
      sync: true
    };
    this.internalFlag &&
      assign(params, {
        serverKey: this.selectPrivateKeyFile,
        serverPass: this.formGroup.value.password,
        serverCertificate: this.selectCertificateFile
      });
    if (this.communicationComponentFlag) {
      assign(params, {
        serverKey: this.selectPrivateKeyFile,
        serverPass: this.formGroup.value.password,
        serverCertificate: this.selectCertificateFile
      });
    }
    if (this.redisComponentFlag) {
      assign(params, {
        serverKey: this.selectPrivateKeyFile,
        serverCertificate: this.selectCertificateFile
      });
    }
    this.check().subscribe(
      response => {
        let warnContent = '';
        if (this.communicationComponentFlag || this.redisComponentFlag) {
          warnContent = this.i18n.get('system_import_certificate_warn_label', [
            find(response, { safety: false })
              ? this.i18n.get('system_cert_safety_tips_label', [
                  toString(
                    without(
                      [response[0]['name'] || '', response[1]['name'] || ''],
                      ''
                    )
                  )
                ])
              : '',
            this.baseUtilService.getProductName()
          ]);
        } else if (this.internalFlag) {
          warnContent = this.i18n.get(
            'system_certificate_push_update_warn_label',
            [
              find(response, { safety: false })
                ? this.i18n.get('system_cert_safety_tips_label', [
                    toString(
                      without(
                        [response[0]['name'] || '', response[1]['name'] || ''],
                        ''
                      )
                    )
                  ])
                : '',
              this.baseUtilService.getProductName()
            ]
          );
        } else {
          warnContent = this.i18n.get('system_import_certificate_ex_label', [
            !response.safety
              ? this.i18n.get('system_cert_safety_tips_label', [
                  response['name'] || ''
                ])
              : ''
          ]);
        }
        this.warningMessageService.create({
          content: warnContent,
          onOK: () => {
            if (this.internalFlag) {
              this.certApiService.pushUpdateCertificate(params).subscribe(
                () => {
                  isFunction(cb) && cb();
                  resolve(true);
                },
                () => {
                  resolve(false);
                }
              );
            } else {
              this.certApiService.importCertificateUsingPOST(params).subscribe(
                () => {
                  isFunction(cb) && cb();
                  resolve(true);
                },
                () => {
                  resolve(false);
                }
              );
            }
          },
          onCancel: () => resolve(false),
          lvAfterClose: result => {
            if (result && result.trigger === 'close') {
              resolve(false);
            }
          }
        });
      },
      () => {
        resolve(false);
      }
    );
  }

  check(): any {
    if (this.internalFlag) {
      return this.check3();
    }

    if (this.communicationComponentFlag || this.redisComponentFlag) {
      return this.check2();
    }

    return this.check1();
  }

  check1() {
    return this.certApiService
      .getCertDetailUsingPOST({
        cert: this.selectCaCertificateFile,
        akOperationTips: false
      })
      .pipe(
        map(res => {
          if (!res.safety) {
            res['name'] = this.selectCaCertificateFile['name'];
          }
          return res;
        })
      );
  }

  check2() {
    return combineLatest([
      this.certApiService.getCertDetailUsingPOST({
        cert: this.selectCaCertificateFile,
        akOperationTips: false
      }),
      this.certApiService.getCertDetailUsingPOST({
        cert: this.selectCertificateFile,
        akOperationTips: false
      })
    ]).pipe(
      map(res => {
        if (!res[0].safety) {
          res[0]['name'] = this.selectCaCertificateFile['name'];
        }

        if (!res[1].safety) {
          res[1]['name'] = this.selectCertificateFile['name'];
        }

        return res;
      })
    );
  }

  check3() {
    return combineLatest([
      this.certApiService.getCertDetailUsingPOST({
        cert: this.selectCaCertificateFile,
        akOperationTips: false
      }),
      this.certApiService.getCertDetailUsingPOST({
        cert: this.selectCertificateFile,
        akOperationTips: false
      })
    ]).pipe(
      map(res => {
        if (!res[0].safety) {
          res[0]['name'] = this.selectCaCertificateFile['name'];
        }

        if (!res[1].safety) {
          res[1]['name'] = this.selectCertificateFile['name'];
        }

        return res;
      })
    );
  }

  ngOnInit() {
    this.initForm();
  }

  uploadChange(e) {
    if (e.activeFiles && e.activeFiles.length) {
      if (e.action === 'remove' && !e.files.length) {
        this.selectCertificateFile = '';
        this.valid$.next(false);
      } else {
        this.selectCertificateFile = e.activeFiles[0].originFile;
        this.valid$.next(true);
      }
    } else {
      this.valid$.next(false);
    }
  }

  uploadCaChange(e) {
    if (e.activeFiles && e.activeFiles.length) {
      if (e.action === 'remove' && !e.files.length) {
        this.selectCaCertificateFile = '';
        this.validCertificate$.next(false);
      } else {
        this.selectCaCertificateFile = e.activeFiles[0].originFile;
        this.validCertificate$.next(true);
      }
    } else {
      this.validCertificate$.next(false);
    }
  }

  privateKeyChange(files) {
    if (!files.length) {
      this.selectPrivateKeyFile = '';
      this.formGroup.get('password').clearValidators();
      this.formGroup.get('password').updateValueAndValidity();
      this.privateKey$.next(false);
    }
  }

  uploadClientChange(e) {
    if (e.activeFiles && e.activeFiles.length) {
      if (e.action === 'remove' && !e.files.length) {
        this.selectAgentCertificate = '';
        this.validAgentCertificate$.next(false);
      } else {
        this.selectAgentCertificate = e.activeFiles[0].originFile;
        this.validAgentCertificate$.next(true);
      }
    } else {
      this.validAgentCertificate$.next(false);
    }
  }

  uploadPrivateKeyChange(e) {
    if (e.activeFiles && e.activeFiles.length) {
      this.selectAgentKey = e.activeFiles[0].originFile;
      this.validAgentKey$.next(true);
    } else {
      this.validAgentKey$.next(false);
    }
  }

  uploadDhparamChange(e) {
    if (e.activeFiles && e.activeFiles.length) {
      if (e.action === 'remove' && !e.files.length) {
        this.selectDhparamCertificate = '';
        this.validDhparam$.next(false);
      } else {
        this.selectDhparamCertificate = e.activeFiles[0].originFile;
        this.validDhparam$.next(true);
      }
    } else {
      this.validDhparam$.next(false);
    }
  }
}
