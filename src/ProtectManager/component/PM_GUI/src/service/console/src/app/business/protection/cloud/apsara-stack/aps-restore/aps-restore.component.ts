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
import { ModalRef } from '@iux/live';
import {
  AppService,
  BaseUtilService,
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  getMultiHostOps,
  I18NService,
  MultiCluster,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType,
  RestoreV2Type,
  VmFileReplaceStrategy
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  each,
  filter,
  find,
  first,
  includes,
  isEmpty,
  isNumber,
  isString,
  isUndefined,
  map,
  toNumber
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-aps-restore',
  templateUrl: './aps-restore.component.html',
  styleUrls: ['./aps-restore.component.less'],
  providers: [CapacityCalculateLabel]
})
export class ApsRestoreComponent implements OnInit {
  resourceData;
  params;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = VmFileReplaceStrategy;
  dataMap = DataMap;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  verifyStatus;
  copyVerifyDisableLabel: string;
  CopyDataVerifyStatus = DataMap.HCSCopyDataVerifyStatus;

  orgOptions = [];
  zoneOptions = [];
  serverOptions = [];
  proxyOptions = [];
  diskData = [];
  newDiskData = [];
  hasSystem = false; // 判断副本内是否包含系统盘
  copySystemDisk;
  needPassword = false;
  isDisableOrigin = false;

  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInput: this.i18n.get('common_invalid_input_label')
  };

  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;

  constructor(
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private appService: AppService,
    private appUtilsService: AppUtilsService,
    private restoreV2Service: RestoreApiV2Service,
    private capacityCalculateLabel: CapacityCalculateLabel,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    const properties = JSON.parse(this.rowCopy.properties);
    this.verifyStatus = properties?.verifyStatus;
    let needRestoreDisks = properties?.volList;
    if (isEmpty(needRestoreDisks)) {
      needRestoreDisks = properties.extendInfo?.volList || [];
    }
    this.copySystemDisk = find(needRestoreDisks, { bootable: 'system' });
    this.hasSystem = !!this.copySystemDisk;
    this.initForm();
    this.getOldDiskData();
    this.initCopyVerifyDisableLabel();
    this.getOrganizationOptions();
    this.getProxyOptions();
    this.getResourceDetail();
  }

  initCopyVerifyDisableLabel() {
    if (
      includes([this.CopyDataVerifyStatus.noGenerate.value], this.verifyStatus)
    ) {
      this.copyVerifyDisableLabel = this.i18n.get(
        'common_generate_verify_file_disable_label'
      );
    }
    if (
      includes([this.CopyDataVerifyStatus.Invalid.value], this.verifyStatus)
    ) {
      this.copyVerifyDisableLabel = this.i18n.get(
        'common_invalid_verify_file_disable_label'
      );
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl({
        value: this.resourceData.name,
        disabled: true
      }),
      password: new FormControl('', {
        validators: [
          this.validPassword(),
          this.baseUtilService.VALID.required()
        ]
      }),
      targetOrg: new FormControl(
        { value: '', disabled: true },
        {
          validators: this.baseUtilService.VALID.required()
        }
      ),
      targetZone: new FormControl('', {
        validators: this.baseUtilService.VALID.required()
      }),
      server: new FormControl('', {
        validators: this.baseUtilService.VALID.required()
      }),
      targetResourceSet: new FormControl(''),
      proxyHost: new FormControl([]),
      restoreAutoPowerOn: new FormControl(false),
      copyVerify: new FormControl(false)
    });
    this.formGroup.get('password').disable();
    this.needPassword = false;

    if (
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) ||
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
      this.isDisableOrigin = true;
    }

    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN) {
      this.formGroup.get('targetOrg').disable();
      this.formGroup.get('targetZone').disable();
      this.formGroup.get('server').disable();
    }

    this.disableOkBtn();
    this.listenForm();
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('targetOrg').disable();
        this.formGroup.get('targetZone').disable();
        this.formGroup.get('server').disable();
        defer(() => this.getResourceDetail());
      } else {
        this.formGroup.get('targetOrg').enable();
        this.formGroup.get('targetZone').enable();
        this.formGroup.get('server').enable();
      }
      this.formGroup.get('password').clearValidators();
      this.needPassword = false;
    });

    this.formGroup.get('targetOrg').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.zoneOptions = [];
      this.getZoneOptions(res);
    });

    this.formGroup.get('targetZone').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.serverOptions = [];
      this.getServerOptions(res);
    });

    this.formGroup.get('server').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      defer(() => this.getResourceDetail());
    });
  }

  validPassword(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup) || isEmpty(control.value)) {
        return null;
      }

      const reg1 = /^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)(?=.*[()`~!@#$%^&*_\-+=|{}\[\]:;<>,.?/]).{8,30}$/;
      if (!reg1.test(control.value)) {
        return { invalidInput: { value: control.value } };
      }
      return null;
    };
  }

  getOldDiskData() {
    const properties = JSON.parse(this.rowCopy.properties || '{}');
    let needRestoreDisks = properties?.volList;
    if (isEmpty(needRestoreDisks)) {
      needRestoreDisks = properties.extendInfo?.volList || [];
    }
    each(needRestoreDisks, item => {
      assign(item, {
        nameId: `${item.name || '--'}(${item.uuid})`,
        volumeType: item.volume_type,
        id: item.uuid,
        size: parseInt(
          this.capacityCalculateLabel.transform(
            item.volSizeInBytes,
            '1.0-0',
            CAPACITY_UNIT.BYTE,
            true
          )
        )
      });
    });
    this.diskData = needRestoreDisks;
  }

  getOrganizationOptions() {
    const extParams = {
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.ApsaraStack.value
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const orgArray = [];
        each(resource, item => {
          orgArray.push({
            ...item,
            key: item.name,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.orgOptions = orgArray;
      }
    );
  }

  getZoneOptions(res) {
    const extParams = {
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.APSZone.value,
        rootUuid: res
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const zoneArray = [];
        each(resource, item => {
          zoneArray.push({
            ...item,
            key: item.name,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.zoneOptions = zoneArray;
      }
    );
  }

  getServerOptions(res) {
    let zone = find(this.zoneOptions, { uuid: res });
    const extParams = {
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.APSCloudServer.value,
        rootUuid: zone.environment.uuid,
        path: [['=~'], zone.path + '/']
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const serverArray = [];
        each(resource, item => {
          serverArray.push({
            ...item,
            key: item.name,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.serverOptions = serverArray;
      }
    );
  }

  getResourceDetail() {
    let data;
    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN) {
      data = this.resourceData;
    } else {
      data = find(this.serverOptions, { uuid: this.formGroup.value.server });
    }
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        queryDependency: true,
        conditions: JSON.stringify({
          uuid: data.rootUuid || data.root_uuid
        })
      })
      .subscribe((res: any) => {
        if (first(res.records)) {
          const onlineAgents = res.records[0]?.dependencies?.agents?.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
          if (isEmpty(onlineAgents)) {
            return;
          }
          const agentsId = onlineAgents[0].uuid;
          this.getDisk(agentsId);
        }
      });
  }

  getDisk(agentsId, recordsTemp?: any[], startPage?: number) {
    let data;
    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN) {
      data = this.resourceData;
    } else {
      data = find(this.serverOptions, { uuid: this.formGroup.value.server });
    }
    const params = {
      agentId: agentsId,
      envId: data.rootUuid || data.root_uuid,
      resourceIds: [data.uuid || data.root_uuid],
      pageNo: startPage || 1,
      pageSize: 200,
      conditions: JSON.stringify({
        resourceType: 'APS-disk',
        uuid: data.uuid,
        regionId: data.extendInfo.regionId
      })
    };

    this.appService.ListResourcesDetails(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = 1;
      }
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / 200) ||
        res.totalCount === 0
      ) {
        each(recordsTemp, item => {
          assign(item, {
            size: item.extendInfo?.size,
            mode: item.extendInfo?.type === 'data' ? 'false' : 'true',
            kinds: item.extendInfo?.category,
            sla: false
          });
        });
        this.newDiskData = recordsTemp;
        // 如果是新建磁盘，则判断恢复到的目标位置有没有系统盘，如果不是新建，则判断覆盖位置是否是系统盘
        const tmpNewSystemDisk = find(this.newDiskData, {
          uuid: this.copySystemDisk.uuid
        });
        if (
          this.hasSystem &&
          ((!!tmpNewSystemDisk && tmpNewSystemDisk.mode === 'true') ||
            (!tmpNewSystemDisk &&
              !find(this.newDiskData, item => item.mode === 'true')))
        ) {
          this.formGroup.get('password').enable();
          this.needPassword = true;
        } else {
          this.formGroup.get('password').disable();
          this.needPassword = false;
        }
        return;
      }
      startPage++;
      this.getDisk(agentsId, recordsTemp, startPage);
    });
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

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      agents: this.formGroup.value.proxyHost,
      targetEnv:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData.environment_uuid
          : this.formGroup.value.targetOrg,
      restoreType:
        this.restoreType === RestoreV2Type.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject:
        this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
          ? this.resourceData.uuid
          : this.formGroup.value.server,
      extendInfo: {
        restoreLevel: 0,
        powerState: this.formGroup.value.restoreAutoPowerOn ? '1' : '0',
        copyVerify: this.formGroup.value.copyVerify ? 'true' : 'false'
      },
      subObjects: map(this.diskData, item => {
        let curData;
        if (!!find(this.newDiskData, { uuid: item.uuid })) {
          curData = {
            id: item.uuid,
            size: toNumber(item.size),
            isNewDisk: String(!find(this.newDiskData, { uuid: item.uuid }))
          };
        } else {
          curData = {
            id: '',
            size: toNumber(item.size),
            isNewDisk: 'true'
          };
        }

        if (item.bootable === 'system' && this.needPassword) {
          assign(curData, {
            password: this.formGroup.get('password').value
          });
        }

        return JSON.stringify({
          uuid: item.uuid,
          extendInfo: {
            targetVolume: JSON.stringify(curData)
          }
        });
      })
    };

    return params;
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? this.resourceData?.name
      : `${
          find(this.serverOptions, {
            value: this.formGroup.value.server
          })['label']
        }`;
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
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
