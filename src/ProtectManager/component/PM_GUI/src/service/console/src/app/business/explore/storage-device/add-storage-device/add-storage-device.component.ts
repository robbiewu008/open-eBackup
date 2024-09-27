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
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedEnvironmentApiService
} from 'app/shared';
import {
  assign,
  cloneDeep,
  defer,
  eq,
  first,
  get,
  isEmpty,
  map,
  remove,
  set
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-storage-device',
  templateUrl: './add-storage-device.component.html',
  styleUrls: ['./add-storage-device.component.less']
})
export class AddStorageDeviceComponent implements OnInit {
  item;
  formGroup: FormGroup;
  isFocusPassword = false;
  deviceStorageType = DataMap.cyberDeviceStorageType;
  pwdVisible = false;
  cyberEngineUserTipsLabel: string =
    'protection_add_storage_user_cyber_engine_tip_label';

  fcCertFilters = [];
  selectFcSiteFile;
  revocationListFilters = [];
  selectRevocationList = '';

  certName = '';
  certSize = '';
  crlName = '';
  crlSize = '';

  typeOptions = this.dataMapService
    .toArray('cyberDeviceStorageType')
    .filter(v => (v.isLeaf = true));

  typeValues = map(
    this.dataMapService.toArray('cyberDeviceStorageType'),
    'value'
  );
  storageDeviceDetectType = DataMap.storageDeviceDetectType;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  deviceNameErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.nameErrorTip
  );
  portErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  });
  usernameErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.lengthErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
    }
  );
  passwordErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    this.baseUtilService.lengthErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
    }
  );
  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private message: MessageService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.initFilters();
    this.updateData();
  }

  copy() {
    return false;
  }

  initForm() {
    this.formGroup = this.fb.group({
      detectType: new FormControl('1', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      type: new FormControl(
        DataMap.cyberDeviceStorageType.OceanStorDorado.value,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      equipment_name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      fqdn: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ]
      }),
      port: new FormControl('8088', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
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
      verify_status: new FormControl(true)
    });

    this.formGroup.statusChanges.subscribe(formGroupStatus => {
      const modalIns = this.modal.getInstance();
      defer(() => {
        modalIns.lvOkDisabled = this.formGroup.value.verify_status
          ? !(formGroupStatus === 'VALID' && !!this.selectFcSiteFile)
          : formGroupStatus !== 'VALID';
      });
    });
    this.formGroup.get('verify_status')?.valueChanges.subscribe(() => {
      this.selectFcSiteFile = ''; // 清空
      this.selectRevocationList = '';
    });

    defer(() => {
      this.setTypeOptionsByDetectType(this.formGroup.get('detectType').value);
    });
  }

  detectTypeChange(val: number) {
    const cyberEngineDefaultPort = '8088';
    const inDeviceDefaultPort = '25081';
    if (eq(val, this.storageDeviceDetectType.cyberEngine.value)) {
      this.formGroup.get('port')?.setValue(cyberEngineDefaultPort);
      this.cyberEngineUserTipsLabel =
        'protection_add_storage_user_cyber_engine_tip_label';
    }
    if (eq(val, this.storageDeviceDetectType.inDevice.value)) {
      this.formGroup.get('port')?.setValue(inDeviceDefaultPort);
      this.cyberEngineUserTipsLabel =
        'protection_add_storage_user_in_device_tip_label';
    }
    this.setTypeOptionsByDetectType(val);
  }

  private setTypeOptionsByDetectType(detectType) {
    if (eq(detectType, this.storageDeviceDetectType.cyberEngine.value)) {
      this.typeOptions = this.dataMapService
        .toArray('cyberDeviceStorageType')
        .filter(v => (v.isLeaf = true));
    } else {
      const _typeOptions = this.typeOptions;
      remove(_typeOptions, option =>
        eq(option.value, DataMap.cyberDeviceStorageType.OceanStorPacific.value)
      );
      this.typeOptions = cloneDeep(_typeOptions);
    }
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
            this.message.error(
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
            this.message.error(
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
            this.message.error(
              this.i18n.get('common_format_error_label', ['crl']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            return validFiles;
          }
          if (files[0].size > 5 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['5KB']),
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
              !this.selectFcSiteFile || this.formGroup.invalid;
          };
          reader.readAsDataURL(files[0].originFile);
          return validFiles;
        }
      }
    ];
  }

  certChange(files) {
    if (isEmpty(files)) {
      this.selectFcSiteFile = '';
      const modalIns = this.modal.getInstance();
      modalIns.lvOkDisabled = !this.selectFcSiteFile || this.formGroup.invalid;
    }
  }

  revocationListChange(files) {
    if (isEmpty(files)) {
      this.selectRevocationList = '';
    }
  }

  updateData() {
    if (!this.item) {
      return;
    }
    const item = {
      type: this.item.subType,
      equipment_name: this.item.name,
      fqdn: this.item.endpoint,
      port: +this.item.port,
      username: this.item.auth?.authKey,
      verify_status: !!+this.item.extendInfo.verifyStatus
    };
    if (this.isCyberEngine) {
      set(item, 'detectType', get(this.item, ['extendInfo', 'detectType']));
    }
    this.formGroup.patchValue(item);
    defer(() => {
      this.modal.getInstance().lvOkDisabled =
        !this.selectFcSiteFile || this.formGroup.invalid;
      if (this.item.name) {
        this.formGroup.get('equipment_name').disable();
      } else {
        this.formGroup.get('equipment_name').enable();
      }
    });
  }

  getParams() {
    return {
      name: !this.item ? this.formGroup.value.equipment_name : this.item.name,
      type: 'StorageEquipment',
      subType: this.formGroup.value.type,
      endpoint: this.formGroup.value.fqdn,
      port: this.formGroup.value.port,
      extendInfo: {
        verifyStatus: this.formGroup.value.verify_status ? '1' : '0',
        detectType: this.isCyberEngine
          ? Number(this.formGroup.value.detectType)
          : void 0,
        snapConsistency: '1'
      },
      auth: {
        authType: 2,
        authKey: this.formGroup.value.username,
        authPwd: this.formGroup.value.password,
        extendInfo: {
          enableCert: String(+this.formGroup.value.verify_status),
          certification: this.selectFcSiteFile,
          revocationlist: this.selectRevocationList
        }
      }
    };
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      let body = this.getParams() as any;
      this.protectedEnvironmentApiService
        .RegisterProtectedEnviroment({
          RegisterProtectedEnviromentRequestBody: body
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }

  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      let body = this.getParams() as any;

      this.protectedEnvironmentApiService
        .UpdateProtectedEnvironment({
          envId: this.item.uuid,
          UpdateProtectedEnvironmentRequestBody: body
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }

  onOK(): Observable<void> {
    return this.item ? this.modify() : this.create();
  }
}
