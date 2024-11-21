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
import { DatePipe } from '@angular/common';
import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  CopiesService,
  DataMap,
  extendParams,
  I18NService,
  ProtectedEnvironmentApiService,
  RestoreV2LocationType,
  RestoreV2Type,
  SYSTEM_TIME
} from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  get,
  includes,
  isEmpty,
  isNil,
  isNumber,
  isString,
  isUndefined,
  map,
  omit,
  reject,
  set,
  size,
  some,
  trim
} from 'lodash';
import { Observable, Observer, of } from 'rxjs';
import { finalize, switchMap, tap } from 'rxjs/operators';

@Component({
  selector: 'aui-ocean-base-restore',
  templateUrl: './ocean-base-restore.component.html',
  styleUrls: ['./ocean-base-restore.component.less'],
  providers: [DatePipe]
})
export class OceanBaseRestoreComponent implements OnInit {
  resourceData;
  backup_type;
  restoreTimeStamp;
  properties;
  isChoosed = [];
  clusterOptions = [];
  resourcePoolOptions = [];
  targetResourcePoolOptions = [];
  originalTenantData = [];
  newTenantData = [];
  originalSelection = [];
  newSelection = [];
  validOptions: boolean[] = [];
  pageSize = CommonConsts.PAGE_SIZE * 5;
  formGroup: FormGroup;
  readonly PAGESIZE = CommonConsts.PAGE_SIZE * 10;
  restoreLocationType = RestoreV2LocationType;
  dataMap = DataMap;
  showTime = {};
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private copiesService: CopiesService,
    private i18n: I18NService,
    private datePipe: DatePipe
  ) {}

  ngOnInit() {
    this.initData();
    this.initForm();
  }

  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      if (!this.formGroup.get('targetLocation')?.value) {
        this.formGroup.get('targetLocation').setValue(config.targetEnv);
      }
      if (config.extendInfo.tenantInfos) {
        try {
          const tenantInfos = JSON.parse(config.extendInfo.tenantInfos);
          this.newSelection = filter(this.newTenantData, item =>
            includes(map(tenantInfos, 'originalId'), item.id)
          );
          this.setTenantInfos(tenantInfos);
        } catch (error) {
          this.newSelection = [];
        }
      }
      setTimeout(() => this.disableOkBtn());
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getClusterOptions(null, null, event);
  }

  setTenantInfos(tenantInfos) {
    each(this.newTenantData, item => {
      const config = find(tenantInfos, { originalId: item.id });
      if (config) {
        assign(item, {
          targetName: item.targetName || config.targetName,
          selectedData: find(this.targetResourcePoolOptions, {
            value: config.resourcePoolId
          })
        });
      }
    });
  }

  initData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.backup_type = this.rowCopy.backup_type;
    this.properties = isString(this.rowCopy.properties)
      ? JSON.parse(this.rowCopy.properties)
      : {};
    if (this.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      this.restoreTimeStamp =
        get(this.rowCopy, 'restoreTimeStamp') ||
        get(this.properties, 'endTime');
      this.restoreTimeStamp = this.datePipe.transform(
        Number(this.restoreTimeStamp) * 1000,
        'yyyy-MM-dd HH:mm:ss'
      );
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl({
        value:
          this.childResType === DataMap.Resource_Type.OceanBaseCluster.value
            ? this.resourceData?.name
            : this.rowCopy.resource_environment_name,
        disabled: true
      }),
      targetLocation: new FormControl('', {
        validators: this.baseUtilService.VALID.required()
      })
    });
    this.listenForm();
    this.modal.getInstance().lvOkDisabled = true;
    if (
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
    }
    this.getClusterOptions();
    this.getCopyTenant(this.properties?.tenant_list);
    if (
      this.formGroup.getRawValue().restoreLocation ===
      RestoreV2LocationType.ORIGIN
    ) {
      this.getTargetResourcePoolOptions(
        this.resourceData?.root_uuid || this.resourceData?.rootUuid,
        RestoreV2LocationType.ORIGIN
      );
    }
  }

  listenForm() {
    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      this.originalSelection = [];
      this.newSelection = [];
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('targetLocation').setValue('');
        this.newTenantData = map(this.newTenantData, item =>
          omit(item, ['selectedData', 'targetName', 'parent'])
        );
        this.targetResourcePoolOptions = [];
      }
    });
    this.formGroup.get('targetLocation').valueChanges.subscribe(res => {
      this.newSelection = [];
      this.validOptions.fill(false);
      this.newTenantData = map(this.newTenantData, item =>
        omit(item, ['selectedData', 'targetName', 'parent'])
      );
      this.getTargetResourcePoolOptions(res, RestoreV2LocationType.NEW);
    });
  }

  getTargetResourcePoolOptions(envId, location) {
    if (!envId) {
      return;
    }
    const extParams = {
      envId,
      conditions: JSON.stringify({ queryType: 'pool' })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params =>
        this.protectedEnvironmentApiService
          .ListEnvironmentResource(params)
          .pipe(
            finalize(() => {
              this.modal.getInstance().lvOkDisabled = true;
            })
          ),
      resource => {
        const clusterInfo = get(resource[0], 'extendInfo.clusterInfo');
        let poolInfo;
        if (!isUndefined(clusterInfo)) {
          poolInfo = JSON.parse(clusterInfo).pools;
        }
        const tmpArr = map(poolInfo, data => {
          return {
            value: data.resource_pool_id,
            key: data.resource_pool_id,
            label: data.resource_pool_name,
            isLeaf: true,
            disabled: false
          };
        });
        if (location === RestoreV2LocationType.NEW) {
          this.targetResourcePoolOptions = cloneDeep(tmpArr);
          this.updateDrillData();
        } else {
          this.resourcePoolOptions = tmpArr;
        }
      }
    );
  }

  getClusterOptions(recordsTemp?, startPage?, labelParams?: any) {
    const conditions = {
      subType: DataMap.Resource_Type.OceanBaseCluster.value
    };
    extendParams(conditions, labelParams);
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify(conditions)
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / this.PAGESIZE) ||
        res.totalCount === 0
      ) {
        recordsTemp = recordsTemp.filter(
          item =>
            item.linkStatus === DataMap.resource_LinkStatus_Special.normal.value
        );
        this.clusterOptions = map(recordsTemp, item => {
          return assign(item, {
            value: item.uuid,
            key: item.uuid,
            label: item.name,
            isLeaf: true
          });
        });
        this.updateDrillData();
        return;
      }
      this.getClusterOptions(recordsTemp, startPage, labelParams);
    });
  }

  checkChange(data?, index?) {
    if (!isUndefined(index) && !isUndefined(data)) {
      this.validOptions[index] = this.validteTenantName(data);
    }
    this.disableOkBtn();
  }

  // 资源池互斥选择
  selectionChange() {
    // 原位置与新位置两个资源池分开控制
    const isOrigin =
      this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN;
    const data = isOrigin ? this.originalTenantData : this.newTenantData;
    const options = isOrigin
      ? this.resourcePoolOptions
      : this.targetResourcePoolOptions;
    const isChoosed = map(data, item => item?.selectedData?.label);
    options.forEach(item => {
      item.disabled = includes(isChoosed, item.label);
      return { ...item };
    });
    isOrigin
      ? (this.resourcePoolOptions = [...options])
      : (this.targetResourcePoolOptions = [...options]);
    this.disableOkBtn();
  }

  checkBoxChange() {
    this.disableOkBtn();
  }

  getOriginalDataFromData(data) {
    return map(data, item => ({
      name: item?.name,
      id: item?.id,
      ...item
    }));
  }

  getCopyTenant(data) {
    // 日志副本时间线恢复时需要过滤不在范围内的数据
    let result = data;
    if (this.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      const params = {
        pageSize: CommonConsts.PAGE_SIZE,
        pageNo: CommonConsts.PAGE_START,
        orders: ['-display_timestamp'],
        conditions: JSON.stringify({
          resource_id: this.rowCopy.resource_id,
          backup_type: [
            DataMap.CopyData_Backup_Type.full.value,
            DataMap.CopyData_Backup_Type.incremental.value
          ]
        })
      };
      const timeStamp =
        get(this.rowCopy, 'restoreTimeStamp') ||
        get(this.properties, 'endTime');
      // 日志副本要先去寻找最近的数据副本，所以这里要查一次数据副本
      // 查出的副本是按时间降序排列的(全量、增量)数据副本
      this.copiesService
        .queryResourcesV1CopiesGet(params)
        .pipe(
          switchMap(res => {
            // 将timestamp大于当前指定时间的数据去除 剩下的首个元素就是最近的数据副本
            const filterRes = reject(
              res.items,
              ({ timestamp }) => Number(timestamp) > timeStamp * 1e6
            );
            if (!isEmpty(filterRes)) {
              const properties = get(first(filterRes), 'properties', '{}');
              result = get(JSON.parse(properties), 'tenant_list', '[]');
            }
            return of(result);
          }),
          tap(result => {
            // 如果请求成功则使用最近数据副本的tenant_list
            this.originalTenantData = this.getOriginalDataFromData(result);
          })
        )
        .subscribe({
          next: result => {
            // 更新
            this.newTenantData = [...this.originalTenantData];
            this.updateDrillData();
          },
          error: error => {
            // 请求失败则使用当前副本的tenant_list
            this.originalTenantData = this.getOriginalDataFromData(data);
            this.newTenantData = [...this.originalTenantData];
            this.updateDrillData();
          }
        });
    } else {
      this.originalTenantData = this.getOriginalDataFromData(data);
      this.newTenantData = [...this.originalTenantData];
      this.updateDrillData();
    }
  }

  getParams() {
    let params;
    const data = {};
    if (this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN) {
      const locationArr = map(this.originalSelection, item => {
        assign(data, {
          originalName: item.name,
          originalId: item.id,
          targetName: item.name,
          resourcePoolId: item.selectedData.value,
          resourcePoolName: item.selectedData.label
        });
        if (
          this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value
        ) {
          assign(data, {
            timestamp: this.restoreTimeStamp
          });
        }
        return { ...data };
      });
      params = {
        copyId: this.rowCopy.uuid,
        restoreType:
          this.restoreType === RestoreV2Type.CommonRestore
            ? RestoreV2Type.CommonRestore
            : RestoreV2Type.FileRestore,
        targetEnv: this.resourceData.environment_uuid,
        targetLocation: RestoreV2LocationType.ORIGIN,
        targetObject: this.resourceData.uuid,
        extendInfo: { tenantInfos: JSON.stringify(locationArr) }
      };
    } else {
      const locationArr = map(this.newSelection, item => {
        assign(data, {
          originalName: item.name,
          originalId: item.id,
          targetName: trim(item.targetName),
          resourcePoolId: item.selectedData.value,
          resourcePoolName: item.selectedData.label
        });
        if (
          this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value
        ) {
          assign(data, {
            timestamp: this.restoreTimeStamp
          });
        }
        return { ...data };
      });
      params = {
        copyId: this.rowCopy.uuid,
        restoreType:
          this.restoreType === RestoreV2Type.CommonRestore
            ? RestoreV2Type.CommonRestore
            : RestoreV2Type.FileRestore,
        targetEnv: this.formGroup.value.targetLocation,
        targetLocation: RestoreV2LocationType.NEW,
        targetObject: this.formGroup.value.targetLocation,
        extendInfo: { tenantInfos: JSON.stringify(locationArr) }
      };
    }

    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      const timeStamp =
        get(this.rowCopy, 'restoreTimeStamp') ||
        get(this.properties, 'endTime');
      set(params, 'extendInfo.restoreTimestamp', timeStamp);
    }

    return params;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
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
    });
  }

  getClusterName() {
    return this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
      ? [
          this.i18n.get('protection_original_tenant_label'),
          this.rowCopy?.resource_environment_name
        ]
      : [
          this.i18n.get('protection_new_tenant_label'),
          find(this.clusterOptions, {
            key: this.formGroup.get('targetLocation').value
          }).label
        ];
  }

  disableOkBtn() {
    if (
      size(this.originalSelection) &&
      this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN
    ) {
      this.modal.getInstance().lvOkDisabled = some(
        this.originalSelection,
        item => {
          return isNil(item.selectedData);
        }
      );
    } else if (
      size(this.newSelection) &&
      this.formGroup.value.restoreLocation === RestoreV2LocationType.NEW
    ) {
      this.modal.getInstance().lvOkDisabled = some(this.newSelection, item => {
        return (
          isNil(item.selectedData) || this.validteTenantName(item.targetName)
        );
      });
    } else {
      this.modal.getInstance().lvOkDisabled = true;
    }
  }

  validteTenantName(name: string) {
    const regex = /^[a-zA-Z_][a-zA-Z0-9_]{0,127}$/;
    return !regex.test(name) || isEmpty(name);
  }
}
