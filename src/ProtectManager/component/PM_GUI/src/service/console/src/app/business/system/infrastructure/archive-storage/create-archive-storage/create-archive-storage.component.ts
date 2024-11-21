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
import { OptionItem } from '@iux/live';
import {
  ALARMTHRESHOLD_TYPE,
  BaseUtilService,
  CommonConsts,
  ComponentRestApiService,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  StoragesApiService
} from 'app/shared';
import {
  assign,
  cloneDeep,
  get,
  includes,
  isEmpty,
  map,
  omit,
  pick
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-archive-storage',
  templateUrl: './create-archive-storage.component.html',
  styleUrls: ['./create-archive-storage.component.less']
})
export class CreateArchiveStorageComponent implements OnInit {
  data;
  formGroup: FormGroup;
  cloudTypeItems: OptionItem[];
  protocolItems: OptionItem[];
  alarmLevelItems: OptionItem[];
  cloudPlatformTypeItems: OptionItem[];
  recoverValueUnits: OptionItem[];
  alarmthresholdType = ALARMTHRESHOLD_TYPE;
  isSkFocus = false;
  certItems = [];
  OBJECT_STORAGE = '3';
  dataMap = DataMap;
  _includes = includes;

  nameLabel = this.i18n.get('common_name_label');
  typeLabel = this.i18n.get('common_type_label');
  protocolLabel = this.i18n.get('common_protocol_label');
  endpointLabel = this.i18n.get('common_endpoint_label');
  basicInfoLabel = this.i18n.get('common_basic_info_label');
  capacityThresholdLabel = this.i18n.get(
    'common_capacity_alarm_threshold_label'
  );
  timesLabel = this.i18n.get('common_time_label');
  percentageLabel = this.i18n.get('common_percentage_label');
  dataBucketLabel = this.i18n.get('common_data_bucket_label');
  sourceDataBucketLabel = this.i18n.get('system_source_data_bucket_label');
  thresholdTypeLabel = this.i18n.get('system_threshold_type_label');
  absoluteValueLabel = this.i18n.get('common_absolute_value_label');
  alarmThresholdLabel = this.i18n.get('common_alarm_threshold_label');
  clearThresholdLabel = this.i18n.get('system_clear_threshold_label');
  triggerThresholdLabel = this.i18n.get('system_trigger_threshold_label');
  alarmLevelLabel = this.i18n.get('system_alarm_severity_label');
  nameErrorTip = assign(
    { ...this.baseUtilService.nameErrorTip },
    { invalidName: this.i18n.get('common_valid_archive_storage_name_label') }
  );
  rangeErrorTip = assign(
    {
      ...this.baseUtilService.integerErrorTip,
      ...this.baseUtilService.requiredErrorTip
    },
    {
      invalidMinSize: this.i18n.get('common_valid_minsize_label', [1]),
      invalidMaxSize: this.i18n.get('common_valid_maxsize_label', [
        1099511627776
      ])
    }
  );
  maxLengthErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256]),
    spaceErrorTip: this.i18n.get('common_valid_space_label')
  };
  urlErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('protection_object_url_invalid_label'),
    spaceErrorTip: this.i18n.get('common_valid_space_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
  };
  portErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  });
  rangeValueLabel = this.i18n.get('1-1099511627776');
  bucketNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    spaceErrorTip: this.i18n.get('common_valid_space_label'),
    invalidMinLength: this.i18n.get('common_valid_length_rang_label', [3, 63]),
    invalidMaxLength: this.i18n.get('common_valid_length_rang_label', [3, 63])
  };

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    public storageApiService: StoragesApiService,
    public cookieService: CookieService,
    public certApiService: ComponentRestApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initOptionItems();
    this.getCertificates();
  }

  initForm() {
    this.formGroup = this.fb.group({
      storageName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.cloudStorageName)
        ]
      }),
      type: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      cloudType: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      connectType: new FormControl(DataMap.azureLinkMode.connection.value),
      port: new FormControl(''),
      endpoint: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256),
          this.validSpace()
        ]
      }),
      useHttps: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      certId: new FormControl(''),
      bucketName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256),
          this.validSpace()
        ]
      }),
      ak: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256),
          this.validSpace()
        ]
      }),
      sk: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(256),
          this.validSpace()
        ]
      }),
      indexBucketName: new FormControl(
        {
          value: '',
          disabled: this.cookieService.isCloudBackup
        },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(256),
            this.validSpace()
          ]
        }
      ),
      proxyEnable: new FormControl(false),
      proxyHostName: new FormControl(''),
      proxyUserName: new FormControl(''),
      proxyUserPwd: new FormControl(''),
      alarmEnable: new FormControl(true),
      alarmThreshold: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.minSize(1),
          this.baseUtilService.VALID.maxSize(1099511627776)
        ]
      }),
      alarmLimitValueUnit: new FormControl(1)
    });

    this.formGroup.get('cloudType').valueChanges.subscribe(res => {
      this.formGroup
        .get('alarmEnable')
        .setValue(
          !includes(
            [
              DataMap.Storage_Cloud_Platform.aws.value,
              DataMap.Storage_Cloud_Platform.azure.value
            ],
            res
          )
        );
      if (res === DataMap.Storage_Cloud_Platform.azure.value) {
        if (
          this.formGroup.get('connectType').value ===
          DataMap.azureLinkMode.connection.value
        ) {
          this.formGroup.get('certId').clearValidators();
          this.formGroup.get('port').clearValidators();
          this.formGroup.get('endpoint').clearValidators();
          this.formGroup.get('useHttps').clearValidators();
          this.formGroup.get('ak').clearValidators();
          this.formGroup.get('indexBucketName').clearValidators();
          this.formGroup.get('proxyHostName').clearValidators();
          this.formGroup.get('proxyUserName').clearValidators();
          this.formGroup.get('proxyUserPwd').clearValidators();
        } else {
          this.formGroup
            .get('port')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 65535)
            ]);
          this.formGroup
            .get('endpoint')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(256),
              this.validSpace()
            ]);
          this.formGroup
            .get('useHttps')
            .setValidators([this.baseUtilService.VALID.required()]);
          this.formGroup
            .get('ak')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(256),
              this.validSpace()
            ]);
          this.formGroup.get('indexBucketName').clearValidators();
          this.formGroup.get('proxyHostName').clearValidators();
          this.formGroup.get('proxyUserName').clearValidators();
          this.formGroup.get('proxyUserPwd').clearValidators();
        }
        this.formGroup
          .get('bucketName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.minLength(3),
            this.baseUtilService.VALID.maxLength(63),
            this.validSpace()
          ]);
      } else {
        this.formGroup
          .get('bucketName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(256),
            this.validSpace()
          ]);
        this.formGroup.get('port').clearValidators();
        this.formGroup
          .get('endpoint')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(256),
            this.validSpace()
          ]);
        this.formGroup
          .get('useHttps')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('ak')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(256),
            this.validSpace()
          ]);
        this.formGroup
          .get('indexBucketName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(256),
            this.validSpace()
          ]);
        if (this.formGroup.value.proxyEnable) {
          this.formGroup
            .get('proxyHostName')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.maxLength(256),
              this.validSpace()
            ]);
          this.formGroup
            .get('proxyUserName')
            .setValidators([
              this.baseUtilService.VALID.maxLength(256),
              this.validSpace()
            ]);
          this.formGroup
            .get('proxyUserPwd')
            .setValidators([this.baseUtilService.VALID.maxLength(256)]);
        } else {
          this.formGroup.get('proxyHostName').clearValidators();
          this.formGroup.get('proxyUserName').clearValidators();
          this.formGroup.get('proxyUserPwd').clearValidators();
        }
      }
      this.formGroup.get('bucketName').updateValueAndValidity();
      this.formGroup.get('certId').updateValueAndValidity();
      this.formGroup.get('port').updateValueAndValidity();
      this.formGroup.get('endpoint').updateValueAndValidity();
      this.formGroup.get('useHttps').updateValueAndValidity();
      this.formGroup.get('ak').updateValueAndValidity();
      this.formGroup.get('indexBucketName').updateValueAndValidity();
      this.formGroup.get('proxyHostName').updateValueAndValidity();
      this.formGroup.get('proxyUserName').updateValueAndValidity();
      this.formGroup.get('proxyUserPwd').updateValueAndValidity();
    });

    this.formGroup.get('connectType').valueChanges.subscribe(res => {
      if (res === DataMap.azureLinkMode.connection.value) {
        this.formGroup.get('port').clearValidators();
        this.formGroup.get('endpoint').clearValidators();
        this.formGroup.get('useHttps').clearValidators();
        this.formGroup.get('ak').clearValidators();
      } else {
        this.formGroup
          .get('port')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.rangeValue(1, 65535)
          ]);
        this.formGroup
          .get('endpoint')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(256),
            this.validSpace()
          ]);
        this.formGroup
          .get('useHttps')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup
          .get('ak')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(256),
            this.validSpace()
          ]);
      }
      this.formGroup.get('port').updateValueAndValidity();
      this.formGroup.get('endpoint').updateValueAndValidity();
      this.formGroup.get('useHttps').updateValueAndValidity();
      this.formGroup.get('ak').updateValueAndValidity();
    });

    this.formGroup.get('useHttps').valueChanges.subscribe(res => {
      if (
        res === '1' &&
        this.formGroup.get('connectType').value ===
          DataMap.azureLinkMode.standard.value
      ) {
        this.formGroup
          .get('certId')
          .setValidators([this.baseUtilService.VALID.required()]);
      } else {
        this.formGroup.get('certId').clearValidators();
      }
      this.formGroup.get('certId').updateValueAndValidity();
    });
    this.formGroup.get('bucketName').valueChanges.subscribe(res => {
      if (!this.cookieService.isCloudBackup) {
        return;
      }
      this.formGroup.get('indexBucketName').setValue(res);
    });

    this.formGroup.get('proxyEnable').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('proxyHostName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(256),
            this.validSpace(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.urlHttpReg)
          ]);
        this.formGroup
          .get('proxyUserName')
          .setValidators([
            this.baseUtilService.VALID.maxLength(256),
            this.validSpace()
          ]);
        this.formGroup
          .get('proxyUserPwd')
          .setValidators([this.baseUtilService.VALID.maxLength(256)]);
      } else {
        this.formGroup.get('proxyHostName').clearValidators();
        this.formGroup.get('proxyUserName').clearValidators();
        this.formGroup.get('proxyUserPwd').clearValidators();
      }
      this.formGroup.get('proxyHostName').updateValueAndValidity();
      this.formGroup.get('proxyUserName').updateValueAndValidity();
      this.formGroup.get('proxyUserPwd').updateValueAndValidity();
    });
    this.formGroup.get('alarmEnable').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('alarmThreshold')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.minSize(0),
            this.baseUtilService.VALID.maxSize(1099511627776)
          ]);
      } else {
        this.formGroup.get('alarmThreshold').clearValidators();
      }
      this.formGroup.get('alarmThreshold').updateValueAndValidity();
    });
    if (!isEmpty(this.data)) {
      this.formGroup.patchValue(this.data);
      this.formGroup.get('useHttps').setValue(this.data.useHttps ? '1' : '0');
      if (this.data.alarmLimitValueUnit) {
        this.formGroup
          .get('alarmLimitValueUnit')
          .setValue(+this.data.alarmLimitValueUnit);
      } else {
        this.formGroup.get('alarmLimitValueUnit').setValue(1);
      }
      if (!this.data.alarmEnable) {
        this.formGroup.get('alarmThreshold').setValue('');
      }
      // 不可编辑项
      this.formGroup.get('storageName').disable();
      this.formGroup.get('endpoint').disable();
      this.formGroup.get('useHttps').disable();
      this.formGroup.get('bucketName').disable();
      this.formGroup.get('indexBucketName').disable();
      this.formGroup.get('proxyHostName').disable();
      this.formGroup.get('proxyUserName').disable();
      this.formGroup.get('proxyUserPwd').disable();
      this.formGroup.get('port').disable();
    }
  }

  validSpace(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      const spaceReg = /\s/;
      if (spaceReg.test(control.value)) {
        return { spaceErrorTip: { value: control.value } };
      }
      return null;
    };
  }

  dealParams(params) {
    if (
      this.formGroup.value.cloudType !==
      DataMap.Storage_Cloud_Platform.azure.value
    ) {
      delete params.connectType;
      delete params.port;
    }
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = cloneDeep(omit(this.formGroup.getRawValue(), ['']));
      if (
        this.formGroup.get('cloudType').value ===
          DataMap.Storage_Cloud_Platform.azure.value &&
        this.formGroup.get('connectType').value ===
          DataMap.azureLinkMode.connection.value
      ) {
        assign(params, {
          useHttps: false
        });
      } else {
        assign(params, {
          useHttps: params.useHttps === '1'
        });
      }

      this.dealParams(params);
      this.storageApiService
        .createStorageUsingPOST1({ request: params as any })
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
      const params = cloneDeep(omit(this.formGroup.getRawValue(), ['']));

      assign(params, {
        useHttps: params.useHttps === '1',
        userName: params.ak,
        password: params.sk,
        certId: params.certId
      });
      const pickItems =
        this.formGroup.value.cloudType ===
        DataMap.Storage_Cloud_Platform.azure.value
          ? [
              'alarmEnable',
              'alarmLimitValueUnit',
              'alarmThreshold',
              'userName',
              'password',
              'connectType',
              'certId'
            ]
          : [
              'alarmEnable',
              'alarmLimitValueUnit',
              'alarmThreshold',
              'userName',
              'password',
              'certId'
            ];
      this.storageApiService
        .updateStorageUsingPUT({
          storageId: this.data.repositoryId,
          request: pick(params, pickItems)
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

  getCertificates() {
    this.certApiService
      .queryComponentsUsingGET({
        akLoading: false
      })
      .subscribe(res => {
        const arr = res.filter(item =>
          get(item, 'type').includes(this.OBJECT_STORAGE)
        );
        this.certItems = map(arr, v => {
          return {
            label: v.name,
            value: v.componentId,
            key: v.componentId,
            isLeaf: true
          };
        });
      });
  }
  initOptionItems() {
    this.cloudTypeItems = this.dataMapService
      .toArray('Archive_Storage_Type')
      .filter((v: OptionItem) => {
        v.isLeaf = true;
        return includes([DataMap.Archive_Storage_Type.s3.value], v.value);
      });
    this.cloudPlatformTypeItems = this.dataMapService
      .toArray('Storage_Cloud_Platform')
      .filter((v: OptionItem) => {
        return (v.isLeaf = true);
      });
    this.alarmLevelItems = this.dataMapService
      .toArray('Alarm_Severity')
      .filter((v: OptionItem) => {
        return (v.isLeaf = true);
      });
    this.protocolItems = [
      {
        value: '1',
        label: 'HTTPS',
        isLeaf: true
      },
      {
        value: '0',
        label: 'HTTP',
        isLeaf: true
      }
    ];
    this.recoverValueUnits = [
      {
        value: 1,
        key: 1,
        label: 'TB',
        isLeaf: true
      },
      {
        value: 2,
        key: 2,
        label: 'GB',
        isLeaf: true
      }
    ];
  }
}
