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
import {
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  NasFileReplaceStrategy,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, each, find, isString, set } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-object-restore',
  templateUrl: './object-restore.component.html',
  styleUrls: ['./object-restore.component.less']
})
export class ObjectRestoreComponent implements OnInit {
  resourceData;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = NasFileReplaceStrategy;
  RestoreType = RestoreType;
  dataMap = DataMap;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  formGroup: FormGroup;
  storageOptions = [];
  bucketOptions = [];
  bucketEnable = false;
  tapeCopy = false;
  bucketErrorTip = {
    invalidName: this.i18n.get('explore_object_bucket_pacific_error_tip_label'),
    invalidSameName: this.i18n.get('explore_object_bucket_same_name_label')
  };
  storageType;

  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  @Output() onStatusChange = new EventEmitter<any>();

  prefixTip = {
    invalidName: this.i18n.get(
      'protection_object_bucket_prefix_letter_tip_label'
    ),
    invalidHead: this.i18n.get(
      'protection_object_bucket_prefix_head_tip_label'
    ),
    invalidNear: this.i18n.get(
      'protection_object_bucket_prefix_near_tip_label'
    ),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256])
  };

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private cdr: ChangeDetectorRef,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit(): void {
    // 判断副本是否是磁带归档，且已开启索引
    this.tapeCopy =
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.tapeArchival.value &&
      this.rowCopy?.indexed === DataMap.CopyData_fileIndex.indexed.value;
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.storageType = Number(this.resourceData.extendInfo.storageType);
    if (this.storageType === DataMap.objectStorageType.ali.value) {
      assign(this.bucketErrorTip, {
        invalidName: this.i18n.get('explore_object_bucket_ali_error_tip_label')
      });
    } else if (this.storageType === DataMap.objectStorageType.hcs.value) {
      assign(this.bucketErrorTip, {
        invalidName: this.i18n.get(
          'explore_object_bucket_hcs_error_tip_basic_label'
        ),
        invalidLine: this.i18n.get(
          'explore_object_bucket_hcs_error_tip_rule_label'
        ),
        invalidAddress: this.i18n.get(
          'explore_object_bucket_hcs_error_tip_ip_label'
        )
      });
    }
    this.initForm();
    this.getStorageOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      isDirectRecovery: new FormControl(this.tapeCopy),
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl({
        value:
          this.resourceData?.environment_name ||
          this.rowCopy?.resource_environment_name ||
          this.rowCopy?.name,
        disabled: true
      }),
      target: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      bucketEnable: new FormControl(false),
      bucketType: new FormControl('0'),
      targetBucket: new FormControl(''),
      newBucket: new FormControl('', {
        validators: this.validBucket()
      }),
      prefix: new FormControl('', {
        validators: [
          this.validPrefix(),
          this.baseUtilService.VALID.maxLength(256)
        ]
      }),
      overwriteType: new FormControl(NasFileReplaceStrategy.Replace, {
        validators: this.baseUtilService.VALID.required()
      })
    });
    this.modal.getInstance().lvOkDisabled = false;

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('target').disable();
      } else {
        this.formGroup.get('target').enable();
        this.getStorageOptions();
      }
    });

    if (
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
      this.modal.getInstance().lvOkDisabled = true;
    }

    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('target').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.getBucketOptions(res);
    });

    this.formGroup.get('bucketType').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      const newBucketControl = this.formGroup.get('newBucket');
      if (res === '0') {
        newBucketControl.clearValidators();
      } else {
        newBucketControl.setValidators([this.validBucket()]);
      }
      newBucketControl.updateValueAndValidity();
    });
  }

  validPrefix(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return;
      }

      const reg1 = /[|:*?<>"\\]+/;
      if (reg1.test(control.value)) {
        return { invalidName: { value: control.value } };
      }
      if (control.value.startsWith('/')) {
        return { invalidHead: { value: control.value } };
      }
      if (control.value.indexOf('//') !== -1) {
        return { invalidNear: { value: control.value } };
      }

      return null;
    };
  }

  validBucket(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return;
      }

      const value = control.value;

      if (find(this.bucketOptions, { value: value })) {
        return { invalidSameName: { value: value } };
      }

      let reg;
      if (this.storageType === DataMap.objectStorageType.pacific.value) {
        reg = /^(?=.*[a-zA-Z])(?=.*\d)[0-9a-zA-Z-_.]{1,255}$/;
      } else if (this.storageType === DataMap.objectStorageType.ali.value) {
        reg = /^(?=[a-z0-9][a-z-0-9]*[a-z0-9])[a-z0-9-]{3,63}$/;
      }

      if (this.storageType === DataMap.objectStorageType.hcs.value) {
        const reg1 = /^[a-z0-9-.]{3,63}$/;
        const reg2 = /^(?![.-])(?!.*\.\.)(?!.*[-.]{2})(?!.*[-.][.-])(?![.-].*[.-])[a-z0-9-.]{3,63}(?<![.-])$/;
        const reg3 = CommonConsts.REGEX.ipv4;
        const reg4 = CommonConsts.REGEX.ipv6;
        if (!reg1.test(value)) {
          return { invalidName: { value: value } };
        }

        if (!reg2.test(value)) {
          return { invalidLine: { value: value } };
        }

        if (reg3.test(value) || reg4.test(value)) {
          return { invalidAddress: { value: value } };
        }
      } else {
        if (!reg.test(value)) {
          return { invalidName: { value: value } };
        }
      }

      return null;
    };
  }

  getStorageOptions() {
    const extParams = {
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.ObjectStorage.value,
        storageType: ['in', Number(this.resourceData.extendInfo.storageType)]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}`,
            isLeaf: true
          });
        });
        this.storageOptions = hostArray;
      }
    );
  }

  getBucketOptions(res) {
    const params: any = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize * 10,
      envId: res
    };

    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        const bucketArray = [];
        each(res.records, item => {
          bucketArray.push({
            value: item.name,
            label: item.name,
            key: item.name,
            isLeaf: true
          });
        });
        this.bucketOptions = bucketArray;
        this.cdr.detectChanges();
      });
  }

  getParams() {
    let target;
    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW) {
      target = find(this.storageOptions, item => {
        return item.value === this.formGroup.value.target;
      });
    }
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData.environment_uuid ||
            this.resourceData.environment?.uuid
          : this.formGroup.value.target,
      restoreType:
        this.restoreType === RestoreType.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData.uuid
          : target.uuid,
      extendInfo: {
        fileReplaceStrategy: String(this.formGroup.value.overwriteType)
      }
    };

    if (this.formGroup.get('isDirectRecovery').value) {
      assign(params, {
        restoreType: RestoreV2Type.FileRestore,
        subObjects: ['/']
      });
    }

    if (this.formGroup.value.bucketEnable) {
      set(
        params,
        'extendInfo.bucketName',
        this.formGroup.value.bucketType === '0'
          ? this.formGroup.value.targetBucket
          : this.formGroup.value.newBucket
      );
      set(
        params,
        'extendInfo.isNewCreateBucket',
        this.formGroup.value.bucketType === '0' ? 'false' : 'true'
      );
    }
    if (
      !!this.formGroup.value?.prefix &&
      this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW
    ) {
      set(params, 'extendInfo.prefix', this.formGroup.value.prefix);
    }
    return params;
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.resourceData?.name
      : `${
          find(this.storageOptions, {
            value: this.formGroup.value.target
          })['label']
        }`;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? {
              name: this.rowCopy.resource_environment_name,
              value: this.resourceData.environment_uuid
            }
          : assign(
              {},
              find(this.storageOptions, {
                value: this.formGroup.value.target
              }),
              {
                name: find(this.storageOptions, {
                  value: this.formGroup.value.target
                })?.label
              }
            ),
      requestParams: this.getParams()
    };
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

  disableOkBtn() {
    if (this.restoreType !== RestoreType.FileRestore) {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    } else {
      this.onStatusChange.emit();
    }
  }
}
