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
  BaseUtilService,
  ClustersApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  StoragePoolService,
  StorageUnitService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, cloneDeep, get, isEmpty, isUndefined } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-storage-unit',
  templateUrl: './create-storage-unit.component.html',
  styleUrls: ['./create-storage-unit.component.less']
})
export class CreateStorageUnitComponent implements OnInit {
  formGroup: FormGroup;
  isEdit: boolean;
  drawData: any;
  isAutoAdded: boolean;
  changedName = true;
  deviceTypeOptions: OptionItem[];
  deviceNameOptions: OptionItem[];
  devicePoolNameOptions: OptionItem[];
  dataMap = DataMap;
  isDistributed = this.appUtilsService.isDistributed;
  isDecouple = this.appUtilsService.isDecouple;

  constructor(
    private baseUtilService: BaseUtilService,
    public fb: FormBuilder,
    public i18n: I18NService,
    public appUtilsService: AppUtilsService,
    public clusterApiService: ClustersApiService,
    public dataMapService: DataMapService,
    private storageUnitService: StorageUnitService,
    private storagePoolService: StoragePoolService
  ) {}

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_storage_pool_name_invalid_label')
  };
  ipErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.ipErrorTip
  };
  portErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  userNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [5, 64])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidNameLength: this.i18n.get('common_valid_length_rang_label', [8, 64])
  };
  thresholdErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 95])
  };

  ngOnInit(): void {
    this.isAutoAdded = this.isEdit && this.drawData.isAutoAdded;
    this.initForm();
    this.initOptionItems();
  }

  validLength(min, max): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || isEmpty(control.value)) {
        return null;
      }
      const value = control.value;
      if (!new RegExp(`^.{${min},${max}}$`).test(value)) {
        return { invalidNameLength: { value: control.value } };
      }
      return null;
    };
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(get(this.drawData, 'name', ''), {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.storagePoolName)
        ]
      }),
      deviceType: new FormControl(get(this.drawData, 'deviceType', ''), {
        validators: [this.baseUtilService.VALID.required()]
      }),
      deviceId: new FormControl(get(this.drawData, 'deviceId', ''), {
        validators: [this.baseUtilService.VALID.required()]
      }),
      poolId: new FormControl(get(this.drawData, 'poolId', ''), {
        validators: [this.baseUtilService.VALID.required()]
      }),
      threshold: new FormControl(get(this.drawData, 'threshold', ''), {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.rangeValue(1, 95),
          this.baseUtilService.VALID.integer()
        ]
      })
    });

    if (this.isAutoAdded) {
      this.formGroup.get('name').clearValidators();
      this.formGroup.get('name').updateValueAndValidity();
    }

    if (!this.isEdit) {
      this.formGroup.get('threshold').clearValidators();
      this.formGroup.get('threshold').updateValueAndValidity();
    }

    this.watchForm();
    if (this.drawData) {
      this.formGroup.get('name').setValue(this.drawData.name);
      if (
        this.drawData?.deviceType === DataMap.poolStorageDeviceType.Server.value
      ) {
        this.formGroup
          .get('deviceType')
          .setValue(DataMap.poolStorageDeviceType.Server.value);
        this.formGroup.patchValue({
          deviceId: this.drawData?.deviceId,
          poolId: this.drawData?.poolId
        });
      }
    }

    if (this.isDistributed && !this.drawData) {
      // 分布式只有服务器
      this.formGroup
        .get('deviceType')
        .setValue(DataMap.poolStorageDeviceType.Server.value);
    }
  }

  initOptionItems() {
    this.deviceTypeOptions = this.dataMapService
      .toArray('poolStorageDeviceType')
      .filter((v: OptionItem) => {
        return (
          (v.isLeaf = true) &&
          v.value !== DataMap.poolStorageDeviceType.OceanPacific.value
        );
      });

    if (this.isDistributed) {
      // 分布式不支持oceanProtectx
      this.deviceTypeOptions = this.deviceTypeOptions.filter(
        item => item.value !== DataMap.poolStorageDeviceType.OceanProtectX.value
      );
    }

    if (!this.isDecouple && !this.isDistributed) {
      // 如果不是这两个就没有服务器类型
      this.deviceTypeOptions.pop();
    }
  }

  watchForm() {
    // 根据设备类型查询存储设备
    this.formGroup.get('deviceType').valueChanges.subscribe(res => {
      this.formGroup.get('deviceId').setValue('');
      this.formGroup.get('poolId').setValue('');
      this.devicePoolNameOptions = [];

      const params = {
        startPage: 0,
        pageSize: 200,
        deviceType: this.formGroup.get('deviceType').value
      };
      this.clusterApiService.getClustersInfoUsingGET(params).subscribe(res => {
        const newArr = [];
        res.records.map(item => {
          // 只展示role=3(备份类型） 且 在线的
          if (item.role === 3 && item.status === 27) {
            newArr.push(
              Object.assign(item, {
                isLeaf: true,
                value: item.storageEsn,
                label: item.clusterName
              })
            );
          }
        });
        this.deviceNameOptions = cloneDeep(newArr);
      });
    });
    // 根据存储设备查询存储池
    this.formGroup.get('deviceId').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }

      const params = {
        deviceId: this.formGroup.get('deviceId').value
      };
      this.storagePoolService.queryStoragePoolGET(params).subscribe(res => {
        const newArr = [];
        res.records.map(item => {
          newArr.push(
            Object.assign(item, {
              isLeaf: true,
              value: item.poolId,
              label: item.name
            })
          );
        });
        this.devicePoolNameOptions = cloneDeep(newArr);
      });
    });
  }

  saveUnitInfo(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) return;
      if (this.isEdit) {
        this.saveEditUnitInfo(observer);
      } else {
        const params = {};
        assign(params, {
          deviceType:
            this.formGroup.get('deviceType').value ===
            DataMap.poolStorageDeviceType.Server.value
              ? 'BasicDisk'
              : this.formGroup.get('deviceType').value,
          name: this.formGroup.get('name').value,
          poolId: this.formGroup.get('poolId').value,
          deviceId: this.formGroup.get('deviceId').value
        });
        this.storageUnitService
          .createBackupUnitPOST({
            createBackupUnitRequest: params
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      }
    });
  }

  saveEditUnitInfo(observer) {
    if (this.isAutoAdded) {
      this.storagePoolService
        .modifyPoolThresholdPUT({
          storagePoolThresholdRequest: {
            deviceId: this.drawData.deviceId,
            threshold: this.formGroup.value.threshold,
            poolId: this.drawData.poolId
          }
        })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: error => {
            observer.error(error);
            observer.complete();
          }
        });
    } else {
      const params = {
        name: this.formGroup.value.name,
        threshold: this.formGroup.value.threshold
      };
      if (
        this.formGroup.get('deviceType').value ===
        DataMap.poolStorageDeviceType.Server.value
      ) {
        // 服务器的判可以修改其他的东西
        assign(params, {
          deviceType:
            this.formGroup.get('deviceType').value ===
            DataMap.poolStorageDeviceType.Server.value
              ? 'BasicDisk'
              : this.formGroup.get('deviceType').value,
          name: this.formGroup.get('name').value,
          poolId: this.formGroup.get('poolId').value,
          deviceId: this.formGroup.get('deviceId').value
        });
      }
      this.storageUnitService
        .modifyStorageUnitPUT({
          id: this.drawData.id,
          updateBackupUnitRequest: params
        })
        .subscribe({
          next: () => {
            observer.next();
            observer.complete();
          },
          error: error => {
            observer.error(error);
            observer.complete();
          }
        });
    }
  }
}
