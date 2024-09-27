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
  FormArray,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService, ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  RestoreV2LocationType,
  RestoreV2Type,
  VmFileReplaceStrategy
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { defer, isString, map, set, join, size, get, each } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-gaussdb-restore',
  templateUrl: './gaussdb-restore.component.html',
  styleUrls: ['./gaussdb-restore.component.less']
})
export class GaussdbRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  dataMap = DataMap;
  formGroup: FormGroup;
  basicFormGroup: FormGroup;
  cloudFormGroup: FormGroup;
  databaseFormGroup: FormGroup;
  activeIndex = 0;
  restoreLocation: any = RestoreV2LocationType.ORIGIN;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = VmFileReplaceStrategy;
  projectOptions = [];
  targetOptions = [];
  disabledOrigin = false;
  readonly NOORDER = 'no order';
  availableIdOptions = [
    {
      key: this.i18n.get('explore_no_order_label'),
      label: this.i18n.get('explore_no_order_label'),
      value: this.NOORDER,
      isLeaf: true
    }
  ];
  paymentOptions = [
    {
      key: this.i18n.get('explore_no_order_label'),
      label: this.i18n.get('explore_no_order_label'),
      value: this.NOORDER,
      isLeaf: true
    },
    {
      key: this.i18n.get('explore_pay_as_you_go_label'),
      label: this.i18n.get('explore_pay_as_you_go_label'),
      value: 'postPaid',
      isLeaf: true
    }
  ];
  utcOptions = [
    {
      key: this.i18n.get('explore_no_order_label'),
      label: this.i18n.get('explore_no_order_label'),
      value: this.NOORDER,
      isLeaf: true
    },
    {
      key: 'UTC-11:00',
      label: 'UTC-11:00',
      value: 'UTC-11:00',
      isLeaf: true
    },
    {
      key: 'UTC-10:00',
      label: 'UTC-10:00',
      value: 'UTC-10:00',
      isLeaf: true
    },
    {
      key: 'UTC-09:00',
      label: 'UTC-09:00',
      value: 'UTC-09:00',
      isLeaf: true
    },
    {
      key: 'UTC-08:00',
      label: 'UTC-08:00',
      value: 'UTC-08:00',
      isLeaf: true
    },
    {
      key: 'UTC-07:00',
      label: 'UTC-07:00',
      value: 'UTC-07:00',
      isLeaf: true
    },
    {
      key: 'UTC-06:00',
      label: 'UTC-06:00',
      value: 'UTC-06:00',
      isLeaf: true
    },
    {
      key: 'UTC-05:00',
      label: 'UTC-05:00',
      value: 'UTC-05:00',
      isLeaf: true
    },
    {
      key: 'UTC-04:00',
      label: 'UTC-04:00',
      value: 'UTC-04:00',
      isLeaf: true
    },
    {
      key: 'UTC-03:00',
      label: 'UTC-03:00',
      value: 'UTC-03:00',
      isLeaf: true
    },
    {
      key: 'UTC-02:00',
      label: 'UTC-02:00',
      value: 'UTC-02:00',
      isLeaf: true
    },
    {
      key: 'UTC-01:00',
      label: 'UTC-01:00',
      value: 'UTC-01:00',
      isLeaf: true
    },
    {
      key: 'UTC-00:00',
      label: 'UTC-00:00',
      value: 'UTC-00:00',
      isLeaf: true
    },
    {
      key: 'UTC+01:00',
      label: 'UTC+01:00',
      value: 'UTC+01:00',
      isLeaf: true
    },
    {
      key: 'UTC+02:00',
      label: 'UTC+02:00',
      value: 'UTC+02:00',
      isLeaf: true
    },
    {
      key: 'UTC+03:00',
      label: 'UTC+03:00',
      value: 'UTC+03:00',
      isLeaf: true
    },
    {
      key: 'UTC+04:00',
      label: 'UTC+04:00',
      value: 'UTC+04:00',
      isLeaf: true
    },
    {
      key: 'UTC+05:00',
      label: 'UTC+05:00',
      value: 'UTC+05:00',
      isLeaf: true
    },
    {
      key: 'UTC+06:00',
      label: 'UTC+06:00',
      value: 'UTC+06:00',
      isLeaf: true
    },
    {
      key: 'UTC+07:00',
      label: 'UTC+07:00',
      value: 'UTC+07:00',
      isLeaf: true
    },
    {
      key: 'UTC+08:00',
      label: 'UTC+08:00',
      value: 'UTC+08:00',
      isLeaf: true
    },
    {
      key: 'UTC+09:00',
      label: 'UTC+09:00',
      value: 'UTC+09:00',
      isLeaf: true
    },
    {
      key: 'UTC+10:00',
      label: 'UTC+10:00',
      value: 'UTC+10:00',
      isLeaf: true
    },
    {
      key: 'UTC+11:00',
      label: 'UTC+11:00',
      value: 'UTC+11:00',
      isLeaf: true
    },
    {
      key: 'UTC+12:00',
      label: 'UTC+12:00',
      value: 'UTC+12:00',
      isLeaf: true
    }
  ];
  resourceData;
  readonly PAGESIZE = CommonConsts.PAGE_SIZE * 10;

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_valid_gaussdb_name_label')
  };
  numberErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.integerErrorTip,
    invalidMinSize: this.i18n.get('common_valid_minsize_label', [0])
  };
  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  basicErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [128])
  };
  dbPwdErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32]),
    invalidMinLength: this.i18n.get('common_valid_minlength_label', [8])
  };
  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    public i18n: I18NService,
    private message: MessageService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.initForm();
    this.getOriginProject();
    this.getProjectOptions();
  }

  getProjectOptions() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.gaussdbForOpengaussProject.value,
        projectId: get(this.resourceData, 'extendInfo.projectId')
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      const projectArray = [];

      each(res.records, item => {
        projectArray.push({
          ...item,
          key: item.uuid,
          value: item.uuid,
          label: item.name,
          isLeaf: true
        });
      });
      this.projectOptions = projectArray;
    });
  }

  getOriginProject() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.gaussdbForOpengaussProject.value,
        uuid:
          this.resourceData?.environment?.uuid ||
          this.resourceData?.environment_uuid
      }),
      akDoException: false
    };
    this.protectedResourceApiService.ListResources(params).subscribe(
      res => {
        this.disabledOrigin =
          this.rowCopy?.resource_status ===
            DataMap.Resource_Status.notExist.value ||
          !size(res.records) ||
          this.rowCopy?.generated_by ===
            DataMap.CopyData_generatedType.cascadedReplication.value;

        if (this.disabledOrigin) {
          this.restoreLocation = RestoreV2LocationType.NEW;
          this.modal.getInstance().lvOkDisabled = true;
          this.formGroup.get('restoreLocation').setValue(this.restoreLocation);

          if (
            this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value
          ) {
            this.restoreLocation = '';
            this.modal.getInstance().lvOkDisabled = true;
            this.formGroup.get('restoreLocation').setValue('');
          }
        }
      },
      error => {
        this.disabledOrigin =
          this.rowCopy?.resource_status ===
            DataMap.Resource_Status.notExist.value ||
          this.rowCopy?.generated_by ===
            DataMap.CopyData_generatedType.cascadedReplication.value;

        if (this.disabledOrigin) {
          this.restoreLocation = RestoreV2LocationType.NEW;
          this.modal.getInstance().lvOkDisabled = true;
          this.formGroup.get('restoreLocation').setValue(this.restoreLocation);

          if (
            this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value
          ) {
            this.restoreLocation = '';
            this.modal.getInstance().lvOkDisabled = true;
            this.formGroup.get('restoreLocation').setValue('');
          }
        }
      }
    );
  }

  validName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return { required: { value: control.value } };
      }

      const value = control.value;
      // 1、只能以字母开头。
      const reg1 = /^[a-zA-Z]/;
      if (!reg1.test(value)) {
        return { invalidName: { value: control.value } };
      }

      // 2、由字母、数字、中文字符、“-”和“_”组成。
      const reg2 = /^[a-zA-Z_0-9-]+$/;
      if (!reg2.test(value)) {
        return { invalidName: { value: control.value } };
      }

      // 3、长度范围是1到64位。
      const reg3 = /^.{4,64}$/;
      if (!reg3.test(value)) {
        return { invalidName: { value: control.value } };
      }

      return null;
    };
  }

  initForm() {
    this.basicFormGroup = this.fb.group({
      targetEnv: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.required(), this.validName()]
      }),
      availableIds: this.fb.array([this.getIdFormGroup()]),
      encode: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(128)
        ]
      }),
      diskType: new FormControl('LOCALSSD'),
      diskCapacity: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.minSize(0)
        ]
      }),
      mainAvailableId: new FormControl(this.NOORDER),
      arbitramentAvailableId: new FormControl(this.NOORDER)
    });
    this.cloudFormGroup = this.fb.group({
      virtualCloudId: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(128)
        ]
      }),
      subnetId: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(128)
        ]
      }),
      securityGroupId: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(128)
        ]
      })
    });
    this.databaseFormGroup = this.fb.group({
      databasePwd: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(8),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      port: new FormControl('8000', {
        validators: [this.baseUtilService.VALID.rangeValue(1, 65535)]
      }),
      paramsGroupId: new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(128)]
      }),
      payment: new FormControl(this.NOORDER),
      backupStart: new FormControl(''),
      backupEnd: new FormControl(''),
      utc: new FormControl(this.NOORDER)
    });
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl({
        value: this.resourceData?.name,
        disabled: true
      }),
      newLocation: this.fb.array([
        this.basicFormGroup,
        this.cloudFormGroup,
        this.databaseFormGroup
      ])
    });

    this.listenForm();
    this.modal.getInstance().lvOkDisabled = false;
  }

  listenForm() {
    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('newLocation').disable();
      } else {
        this.formGroup.get('newLocation').enable();
      }

      defer(() => {
        this.formGroup.updateValueAndValidity();
      });
    });

    this.basicFormGroup.get('availableIds').valueChanges.subscribe(res => {
      defer(() => {
        this.availableIdOptions = [
          {
            key: this.i18n.get('explore_no_order_label'),
            label: this.i18n.get('explore_no_order_label'),
            value: this.NOORDER,
            isLeaf: true
          }
        ];
        if (this.basicFormGroup.get('availableIds').valid) {
          this.availableIdOptions = [
            ...this.availableIdOptions,
            ...map(this.basicFormGroup.value.availableIds, item => {
              return {
                key: item.availableId,
                label: item.availableId,
                value: item.availableId,
                isLeaf: true
              };
            })
          ];
        }
      });
    });
  }

  getIdFormGroup() {
    return this.fb.group({
      availableId: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
  }

  addRow() {
    (this.basicFormGroup.get('availableIds') as FormArray).push(
      this.getIdFormGroup()
    );
  }

  removeRow(i) {
    (this.basicFormGroup.get('availableIds') as FormArray).removeAt(i);
  }

  get availableIds() {
    return (this.basicFormGroup.get('availableIds') as FormArray).controls;
  }

  changeLocation(val) {
    this.formGroup.get('restoreLocation').setValue(val);
  }

  clickWizard(index) {
    const formArr = [
      this.basicFormGroup,
      this.cloudFormGroup,
      this.databaseFormGroup
    ];

    if (index <= this.activeIndex) {
      this.activeIndex = index;
      return;
    }

    if (formArr[this.activeIndex].invalid) {
      formArr[this.activeIndex].markAllAsTouched();
      return;
    }

    this.activeIndex = index;
  }

  disabledMinute(hour, min) {
    return true;
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData?.environment_uuid ||
            this.resourceData?.environment?.uuid
          : this.basicFormGroup.value.targetEnv,
      restoreType: RestoreV2Type.CommonRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject: this.resourceData?.uuid
    };
    const newInstance = {
      instanceId: this.resourceData.uuid,
      backupId: this.rowCopy.uuid,
      name: this.basicFormGroup.value.name,
      availabilityZone: join(
        map(this.basicFormGroup.value.availableIds, 'availableId'),
        ','
      ),
      masterAz:
        this.basicFormGroup.value.mainAvailableId === this.NOORDER
          ? ''
          : this.basicFormGroup.value.mainAvailableId,
      arbitrationAz:
        this.basicFormGroup.value.arbitramentAvailableId === this.NOORDER
          ? ''
          : this.basicFormGroup.value.arbitramentAvailableId,
      flavorRef: this.basicFormGroup.value.encode,
      volumeType: this.basicFormGroup.value.diskType,
      volumeSize: this.basicFormGroup.value.diskCapacity,
      vpcId: this.cloudFormGroup.value.virtualCloudId,
      subnetId: this.cloudFormGroup.value.subnetId,
      securityGroupId: this.cloudFormGroup.value.securityGroupId,
      password: this.databaseFormGroup.value.databasePwd,
      port: this.databaseFormGroup.value.port,
      configurationId: this.databaseFormGroup.value.paramsGroupId,
      chargeInfo:
        this.databaseFormGroup.value.payment === this.NOORDER
          ? ''
          : this.databaseFormGroup.value.payment,
      backupStrategyStartTime:
        this.databaseFormGroup.value.backupStart &&
        this.databaseFormGroup.value.backupEnd
          ? `${this.databaseFormGroup.value.backupStart}-${this.databaseFormGroup.value.backupEnd}`
          : '',
      timeZone:
        this.databaseFormGroup.value.utc === this.NOORDER
          ? this.databaseFormGroup.value.utc
          : ''
    };

    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW) {
      set(params, 'extendInfo', newInstance);
    }

    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      set(params, 'extendInfo.restoreTimestamp', this.rowCopy.restoreTimeStamp);
    }

    return params;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? {
              name: this.formGroup.value.originLocation,
              value:
                this.resourceData?.environment_uuid || this.resourceData?.uuid
            }
          : this.basicFormGroup.value.name,
      restoreLocation: this.formGroup.value.restoreLocation,
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.resourceData?.name
      : this.basicFormGroup.value.name;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
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
}
