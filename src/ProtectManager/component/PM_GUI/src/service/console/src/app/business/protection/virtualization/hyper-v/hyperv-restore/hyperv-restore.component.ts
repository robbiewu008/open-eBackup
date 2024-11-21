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
  FormGroup
} from '@angular/forms';
import {
  AppService,
  BaseUtilService,
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  DatastoreType,
  I18NService,
  ProtectedResourceApiService,
  ResourceType,
  RestoreApiV2Service,
  RestoreV2LocationType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  every,
  find,
  get,
  includes,
  isEmpty,
  isNumber,
  map,
  set
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-hyperv-restore',
  templateUrl: './hyperv-restore.component.html',
  styleUrls: ['./hyperv-restore.component.less']
})
export class HypervRestoreComponent implements OnInit {
  rowCopy;
  childResType;
  restoreType;

  formGroup: FormGroup;
  newFormGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  datastoreType = DatastoreType;
  restoreToNewLocationOnly = false;
  resourceProp;
  originalResource;
  originLocation;
  isEn = this.i18n.isEn;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  unitconst = CAPACITY_UNIT;
  dataMap = DataMap;
  isHyperVCluster = false;
  showHostSelect = false;
  environmentOptions = [];
  hostOptions = [];
  targetVmOptions = [];
  treeData = [];
  targetDatastoreOptions = [];
  networkOptions = [];
  storageDiskTableData = [];
  networkData = [];
  VM_PATH_MAX_LEN = 150;
  valid$ = new Subject<boolean>();

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_valid_name_label'),
    invalidSameName: this.i18n.get('common_duplicate_name_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [80]),
    invalidSpecialName: this.i18n.get('protection_invalid_nasshare_name_label'),
    invalidPeriodName: this.i18n.get('protection_invalid_hyper_vm_name_label')
  };

  vmStorageErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get(
      'protection_restore_hyperv_vm_path_max_len_label'
    )
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private appService: AppService,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initData();
    this.initForm();
    this.getResourceProp();
    this.getDisk();
    this.getEnvironmentOptions();
  }

  initData() {
    this.resourceProp = JSON.parse(this.rowCopy.resource_properties);
    this.isHyperVCluster =
      this.resourceProp.environment_sub_type ===
      DataMap.Resource_Type.hyperVCluster.value;
  }

  initForm() {
    this.originLocation = this.resourceProp.path;
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) ||
      this.rowCopy.is_replicated ||
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value;
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(80),
          this.validSpecialName()
        ]
      }),
      environment: new FormControl(''),
      host: new FormControl(''),
      targetVm: new FormControl(''),
      storage: new FormControl(DatastoreType.DIFFERENT),
      startupNetworkAdaptor: new FormControl(false),
      powerOn: new FormControl(true),
      deleteOriginalVM: new FormControl(false),
      addToCluster: new FormControl(false)
    });
    this.newFormGroup = this.fb.group({
      // 这两个参数独立出来，长度超过时不会影响确认下发，但不能为空
      vmStorage: new FormControl(''),
      targetDatastore: new FormControl('')
    });
    if (this.rowCopy.diskRestore) {
      this.formGroup.get('name').clearValidators();
    }
    this.listenForm();
    setTimeout(() => {
      if (this.restoreToNewLocationOnly) {
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
      }
    });
  }
  validSpecialName() {
    return (control: AbstractControl): { [key: string]: { value: any } } => {
      const regex = /[\\/:*?"<>|]/;
      const regex2 = / *\.$/;
      if (regex.test(control.value)) {
        return { invalidSpecialName: { value: control.value } };
      } else if (regex2.test(control.value)) {
        return { invalidPeriodName: { value: control.value } };
      }
      return null;
    };
  }

  listenForm() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      this.formGroup.get('environment').setValue('');
      if (res === RestoreV2LocationType.ORIGIN) {
        this.isHyperVCluster =
          this.resourceProp.environment_sub_type ===
          DataMap.Resource_Type.hyperVCluster.value;
        this.formGroup.get('environment').clearValidators();
        this.formGroup.get('host').clearValidators();
        this.formGroup.get('targetVm').clearValidators();
        this.newFormGroup.get('vmStorage').clearValidators();
        this.newFormGroup.get('targetDatastore').clearValidators();
      } else {
        this.formGroup
          .get('environment')
          .setValidators([this.baseUtilService.VALID.required()]);
        if (this.showHostSelect) {
          this.formGroup
            .get('host')
            .setValidators([this.baseUtilService.VALID.required()]);
        }
        if (this.rowCopy.diskRestore) {
          this.formGroup
            .get('targetVm')
            .setValidators([this.baseUtilService.VALID.required()]);
        }
        if (!this.rowCopy.diskRestore) {
          if (this.formGroup.value.storage === DatastoreType.DIFFERENT) {
            this.newFormGroup
              .get('vmStorage')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.maxLength(this.VM_PATH_MAX_LEN)
              ]);
            this.newFormGroup.get('targetDatastore').clearValidators();
          } else {
            this.newFormGroup.get('vmStorage').clearValidators();
            this.newFormGroup
              .get('targetDatastore')
              .setValidators([
                this.baseUtilService.VALID.required(),
                this.baseUtilService.VALID.maxLength(this.VM_PATH_MAX_LEN)
              ]);
          }
        }
      }
      this.formGroup.get('environment').updateValueAndValidity();
      this.formGroup.get('host').updateValueAndValidity();
      this.formGroup.get('targetVm').updateValueAndValidity();
      this.newFormGroup.get('vmStorage').updateValueAndValidity();
      this.newFormGroup.get('targetDatastore').updateValueAndValidity();
      defer(() => this.validDatastore());
    });
    this.formGroup.get('environment').valueChanges.subscribe(res => {
      const selectedEnv = find(this.environmentOptions, { uuid: res });
      this.showHostSelect =
        res && selectedEnv?.subType !== DataMap.Resource_Type.hyperVHost.value;
      this.isHyperVCluster =
        res &&
        selectedEnv?.subType === DataMap.Resource_Type.hyperVCluster.value;
      if (this.showHostSelect) {
        this.formGroup
          .get('host')
          .setValidators([this.baseUtilService.VALID.required()]);
        defer(() => this.getHost());
      } else {
        this.formGroup.get('host').clearValidators();
        this.getTargetDatastore(
          res,
          find(this.environmentOptions, { uuid: res })?.rootUuid
        );
        if (this.rowCopy.diskRestore) {
          this.getVm(res);
        }
      }
      this.formGroup.get('host').updateValueAndValidity();
    });
    this.formGroup.get('host').valueChanges.subscribe(res => {
      this.getTargetDatastore(
        res,
        find(this.hostOptions, { uuid: res })?.rootUuid
      );
      if (this.rowCopy.diskRestore) {
        this.getVm(res);
      }
    });
    this.formGroup.get('storage').valueChanges.subscribe(res => {
      if (res === DatastoreType.DIFFERENT) {
        this.newFormGroup
          .get('vmStorage')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(this.VM_PATH_MAX_LEN)
          ]);
        this.newFormGroup.get('targetDatastore').clearValidators();
      } else {
        this.newFormGroup.get('vmStorage').clearValidators();
        this.newFormGroup
          .get('targetDatastore')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(this.VM_PATH_MAX_LEN)
          ]);
      }
      this.newFormGroup.get('vmStorage').updateValueAndValidity();
      this.newFormGroup.get('targetDatastore').updateValueAndValidity();
      defer(() => this.validDatastore());
    });

    this.newFormGroup.valueChanges.subscribe(res => {
      this.validDatastore();
    });
  }

  getResourceProp() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.resourceProp?.uuid,
        akDoException: false
      })
      .subscribe(
        res => {
          this.originalResource = res;
          this.originLocation = this.originalResource.path;
        },
        () => {
          this.originalResource = this.resourceProp;
        }
      );
  }

  getDisk() {
    const resourcePro = JSON.parse(this.rowCopy.properties || '{}');
    const volList = get(resourcePro, 'volList', []);
    this.storageDiskTableData = map(volList, item => {
      item.targetDatastore = new FormControl('', {
        validators: [this.baseUtilService.VALID.maxLength(this.VM_PATH_MAX_LEN)]
      });
      item.targetDatastore.valueChanges.subscribe(() => this.validDatastore());
      let extendInfo = {};
      try {
        extendInfo = JSON.parse(item.extendInfo);
      } catch (e) {}
      return { ...item, extendInfo };
    });
  }

  getEnvironmentOptions(recordsTemp?: any[], startPage?: number) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: 200,
        conditions: JSON.stringify({
          type: ResourceType.Virtualization,
          subType: [
            DataMap.Resource_Type.hyperVScvmm.value,
            DataMap.Resource_Type.hyperVCluster.value,
            DataMap.Resource_Type.hyperVHost.value
          ]
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          this.environmentOptions = map(recordsTemp, item => {
            return {
              ...item,
              label: item.name,
              isLeaf: true
            };
          });
          return;
        }
        this.getEnvironmentOptions(recordsTemp, startPage);
      });
  }

  getHost(recordsTemp?: any[], startPage?: number) {
    if (!this.formGroup.value.environment) {
      return;
    }
    this.protectedResourceApiService
      .ListResources({
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: 200,
        conditions: JSON.stringify({
          rootUuid: this.formGroup.value.environment,
          subType: [DataMap.Resource_Type.hyperVHost.value]
        })
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / 200) ||
          res.totalCount === 0
        ) {
          this.hostOptions = map(recordsTemp, item => {
            return {
              ...item,
              label: item.name,
              isLeaf: true
            };
          });
          return;
        }
        this.getHost(recordsTemp, startPage);
      });
  }

  getVm(hostId) {
    if (!hostId) {
      return;
    }
    const extParams = {
      conditions: JSON.stringify({
        parentUuid: hostId,
        subType: [DataMap.Resource_Type.hyperVVm.value]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.targetVmOptions = map(resource, item => {
          return assign(item, {
            label: item.name,
            isLeaf: true
          });
        });
      }
    );
  }

  getTargetDatastore(resourceId, enId) {
    if (resourceId || enId) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({
        resourceId: resourceId
      })
      .subscribe((res: any) => {
        const onlineAgents = res?.dependencies?.agents;
        if (!isEmpty(onlineAgents)) {
          const agentId = onlineAgents[0].uuid;
          this.getResourcesDetails(agentId, enId);
        }
      });
  }

  getResourcesDetails(agentId, envId, recordsTemp?: any[], startPage?: number) {
    this.appService
      .ListResourcesDetails({
        agentId,
        envId,
        pageNo: startPage || CommonConsts.PAGE_START + 1,
        pageSize: CommonConsts.PAGE_SIZE
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START + 1;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage ===
            Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) + 1 ||
          res.totalCount === 0
        ) {
          this.targetDatastoreOptions = map(recordsTemp, item => {
            return {
              ...item,
              value: item.name,
              label: item.name,
              isLeaf: true
            };
          });
          each(this.storageDiskTableData, item => {
            assign(item, {
              targetDatastoreOptions: [...this.targetDatastoreOptions]
            });
          });
          this.storageDiskTableData = [...this.storageDiskTableData];
          return;
        }
        this.getResourcesDetails(agentId, envId, recordsTemp, startPage);
      });
  }

  validDatastore() {
    // 路径长度超过150需要给出超长提示，但是不能影响点击确认
    // 不同存储时校验 VM配置文件是否不为空
    // 相同存储时校验 目标数据存储是否不为空
    const vmStorageValid = !isEmpty(this.newFormGroup.value.vmStorage);
    const targetDatastoreValid = !isEmpty(
      this.newFormGroup.value.targetDatastore
    );
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      this.valid$.next(true);
    } else {
      if (this.formGroup.value.storage === DatastoreType.DIFFERENT) {
        this.valid$.next(
          every(
            this.storageDiskTableData,
            item => !isEmpty(item.targetDatastore.value)
          ) && vmStorageValid
        );
      } else {
        this.valid$.next(targetDatastoreValid);
      }
    }
  }

  getRestoreLocation(): string {
    let path = '';
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      path = this.resourceProp?.path;
    } else {
      const options = this.showHostSelect
        ? this.hostOptions
        : this.environmentOptions;
      const uuid = this.showHostSelect
        ? this.formGroup.value.host
        : this.formGroup.value.environment;
      path = find(options, { uuid })?.path;
    }
    return path.replace(/\/[^\/]*$/, '');
  }

  getTargetParams() {
    const params = this.getParams();
    assign(params, {
      targetObject:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resourceProp?.uuid
          : find(this.targetVmOptions, { uuid: this.formGroup.value.targetVm })
              ?.uuid
    });
    assign(params.extendInfo, {
      restoreLevel: '1'
    });
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW) {
      const targetVm = find(this.targetVmOptions, {
        uuid: this.formGroup.value.targetVm
      });
      assign(params.extendInfo, {
        restoreTargetPath: targetVm?.extendInfo?.ConfigurationLocation,
        restoreLocation: targetVm?.path
      });
    } else {
      assign(params.extendInfo, {
        restoreLocation: this.originalResource?.path
      });
    }
    return {
      formGroupValue: cloneDeep(this.formGroup.value),
      requestParams: params,
      targetLoacetionPath:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.originalResource.path
          : this.showHostSelect
          ? `${
              find(this.targetVmOptions, {
                uuid: this.formGroup.value.targetVm
              })?.path
            }`
          : `${
              find(this.environmentOptions, {
                uuid: this.formGroup.value.environment
              })?.path
            }`,
      targetDisk:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? JSON.parse(this.originalResource.extendInfo?.disks || '[]')
          : JSON.parse(
              find(this.targetVmOptions, {
                uuid: this.formGroup.value.targetVm
              })?.extendInfo?.disks || '[]'
            )
    };
  }

  getParams() {
    const params = {
      copyId: this.rowCopy?.uuid,
      restoreType: this.restoreType,
      targetEnv:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resourceProp?.root_uuid
          : this.formGroup.value.environment,
      targetLocation: this.formGroup.value.restoreTo,
      targetObject: this.formGroup.value.name,
      extendInfo: {
        deleteOriginVM: '0',
        powerState: this.formGroup.value.powerOn ? '1' : '0',
        restoreLevel: '0',
        disableNetwork: this.formGroup.value.startupNetworkAdaptor ? '0' : '1',
        restoreLocation: this.getRestoreLocation()
      }
    };
    if (this.isHyperVCluster) {
      set(
        params,
        'extendInfo.addToCluster',
        this.formGroup.value.addToCluster ? '1' : '0'
      );
    }
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      assign(params.extendInfo, {
        deleteOriginVM: this.formGroup.value.deleteOriginalVM ? '1' : '0',
        restoreTargetPath: this.originalResource?.extendInfo
          ?.ConfigurationLocation,
        targetHostId: this.resourceProp?.parent_uuid // 原位置发root_uuid有问题，应该发副本对应的主机id
      });
      assign(params, {
        subObjects: map(this.storageDiskTableData, item => {
          return JSON.stringify({
            uuid: item.uuid,
            extendInfo: {
              Path: item.extendInfo?.Path?.replace(/\/[^\/]*$/, '')
            }
          });
        })
      });
    } else {
      // 如果本身是主机，showHostSelect为false，则直接发送uuid
      // 如果是集群，showHostSelect为true，则需要选下面的主机
      // 所以新位置这里和targetEnv保持一致
      assign(params.extendInfo, {
        targetHostId: this.showHostSelect
          ? this.formGroup.value.host
          : this.formGroup.value.environment
      });
      if (this.formGroup.value.storage === DatastoreType.DIFFERENT) {
        assign(params.extendInfo, {
          restoreTargetPath: this.newFormGroup.value.vmStorage
        });
        assign(params, {
          subObjects: map(this.storageDiskTableData, item => {
            return JSON.stringify({
              uuid: item.uuid,
              extendInfo: {
                Path: item.targetDatastore.value
              }
            });
          })
        });
      } else {
        assign(params.extendInfo, {
          restoreTargetPath: this.newFormGroup.value.targetDatastore
        });
        assign(params, {
          subObjects: map(this.storageDiskTableData, item => {
            return JSON.stringify({
              uuid: item.uuid,
              extendInfo: {
                Path: this.newFormGroup.value.targetDatastore
              }
            });
          })
        });
      }
    }
    return params;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
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
