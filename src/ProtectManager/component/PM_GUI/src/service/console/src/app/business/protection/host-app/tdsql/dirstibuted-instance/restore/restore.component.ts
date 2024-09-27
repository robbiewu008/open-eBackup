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
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CapacityCalculateLabel,
  ClientManagerApiService,
  DataMap,
  extendParams,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import {
  assign,
  each,
  filter,
  find,
  first,
  get,
  includes,
  intersection,
  isEmpty,
  isString,
  map,
  reject,
  size
} from 'lodash';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-restore',
  templateUrl: './restore.component.html',
  styleUrls: ['./restore.component.less'],
  providers: [CapacityCalculateLabel]
})
export class RestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  dataMap = DataMap;
  resourceData;
  nodeOptions;
  machineOptions;
  clusterOptions;
  data;
  showAdvanced = {}; // {original:bool,new:bool} 高级参数是否展示
  formGroupValidateInfo = {}; // 当前界面的校验信息，恢复位置切换时需要同步更新。
  originalMachineOptions = [];
  originalDataNodes;
  proxyOptions = []; // 代理主机
  cacheProxyOptions; // 代理主机选线的备份；
  originalEnvId;
  originalDRNode;
  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private capacityCalculate: CapacityCalculateLabel,
    private appUtilsService: AppUtilsService,
    private baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private clientManagerApiService: ClientManagerApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit(): void {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.originalEnvId =
      this.resourceData?.parent_uuid || this.resourceData?.parentUuid;
    this.initForm();
    this.getClusterOptions();
    this.getProxyOptions();
    this.updateDrillData();
  }

  // 演练数据回显
  updateDrillData() {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      const config = this.rowCopy?.drillRecoveryConfig;
      this.formGroup.get('target').setValue(config.targetEnv);
      this.formGroup.get('drMode').setValue(config.extendInfo?.drMode);
    }
  }

  updateTable(event?) {
    // 根据筛选条件更新表格
    this.getClusterOptions(event);
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      drMode: new FormControl('', this.baseUtilService.VALID.required()),
      target: new FormControl(''),
      dataNodes: this.fb.array([], this.baseUtilService.VALID.required()),
      machineInfo: new FormGroup({
        machine: new FormControl(''),
        cpu: new FormControl(''),
        memory: new FormControl(''),
        dataDisk: new FormControl(''),
        logDisk: new FormControl('')
      })
    });
    this.listenForm();
    if (
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value
    ) {
      this.formGroup.get('restoreLocation').setValue(RestoreV2LocationType.NEW);
    }
    if (
      this.formGroup.get('restoreLocation').value ===
      RestoreV2LocationType.ORIGIN
    ) {
      this.getMachineAndHostById(
        this.originalEnvId,
        RestoreV2LocationType.ORIGIN
      );
    }
  }

  listenForm() {
    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      this.formGroup.get('machineInfo').reset();
      this.formGroup.get('machineInfo').updateValueAndValidity();
      this.formGroup.get('drMode').setValue('');
      this.formGroup.get('drMode').updateValueAndValidity();
      if (res === RestoreV2LocationType.ORIGIN) {
        this.showAdvanced[RestoreV2LocationType.NEW] = false;
        this.machineOptions = [...this.originalMachineOptions];
        this.nodeOptions = [...this.originalDRNode];
        this.formGroup.get('target').setValue('');
        this.formGroup.get('target').clearValidators();
        this.formGroup.get('target').updateValueAndValidity();
        this.getMachineAndHostById(
          this.originalEnvId,
          RestoreV2LocationType.ORIGIN
        );
      } else {
        const dataNodesForm = this.formGroup.get('dataNodes') as FormArray;
        dataNodesForm.clear();
        this.nodeOptions = [];
        this.formGroup
          .get('target')
          .setValidators(this.baseUtilService.VALID.required());
        this.formGroup.get('target').updateValueAndValidity();
      }
    });
    this.formGroup.get('machineInfo.machine').valueChanges.subscribe(res => {
      if (!res) return;
      // 返回值是{machine,cpu,memory,dataDisk,logDisk}
      const { machine, ...extraMachineParams } = res;
      each(extraMachineParams, (value, key) => {
        const endValue = value / 1000;
        const startNumber = endValue > 1 ? 1 : 0;
        this.formGroupValidateInfo[key] = {
          rangePlaceholder:
            key === 'cpu'
              ? this.i18n.get('protection_input_range_tips_label', [1, value])
              : this.i18n.get('protection_input_range_tips_label', [
                  startNumber,
                  endValue
                ]),
          rangeErrorTips: {
            ...this.baseUtilService.rangeErrorTip,
            invalidRang:
              key === 'cpu'
                ? this.i18n.get('common_valid_rang_label', [1, value])
                : this.i18n.get('common_valid_rang_label', [
                    startNumber,
                    endValue
                  ])
          },
          validateFn:
            key === 'cpu'
              ? [
                  this.baseUtilService.VALID.integer(),
                  this.baseUtilService.VALID.rangeValue(1, value)
                ]
              : [this.baseUtilService.VALID.rangeValue(startNumber, endValue)]
        };
        this.formGroup.get(['machineInfo', key]).reset();
        this.formGroup
          .get(['machineInfo', key])
          .setValidators([...this.formGroupValidateInfo[key].validateFn]);
      });
    });
    this.formGroup.get('target').valueChanges.subscribe(res => {
      if (
        !!res &&
        this.formGroup.get('restoreLocation').value ===
          RestoreV2LocationType.NEW
      ) {
        this.formGroup.get('drMode').updateValueAndValidity();
        this.nodeOptions = [];
        this.showAdvanced[RestoreV2LocationType.NEW] = false;
        this.formGroup.get('machineInfo.machine').reset();
        this.formGroup.get('machineInfo.machine').updateValueAndValidity();
        this.getMachineAndHostById(res, RestoreV2LocationType.NEW);
      }
    });
    this.formGroup.get('drMode').valueChanges.subscribe(res => {
      if (!res) {
        this.addDataRow([]);
      } else {
        this.addDataRow(new Array(res));
      }
    });

    this.formGroup.get('dataNodes').valueChanges.subscribe(res => {
      if (!res || isEmpty(res)) {
        this.proxyOptions = map(this.proxyOptions, item =>
          assign(item, {
            disabled: false
          })
        );
        return;
      }
      const cacheArr = map(res, 'parentUuid');
      this.proxyOptions = map(this.proxyOptions, item =>
        assign(item, {
          disabled: includes(cacheArr, item.uuid)
        })
      );
    });
  }

  get dataNodes() {
    return (this.formGroup.get('dataNodes') as FormArray).controls;
  }

  setNode(index: number) {
    if (this.isDrill && !isEmpty(this.rowCopy?.drillRecoveryConfig)) {
      let parentUuid = '';
      try {
        const restoreHosts = JSON.parse(
          this.rowCopy?.drillRecoveryConfig.extendInfo?.restoreHosts
        );
        parentUuid = restoreHosts[index]?.parentUuid;
      } catch (error) {
        parentUuid = '';
      }
      return parentUuid;
    }
    return '';
  }

  addDataRow(rowData: any[]) {
    const dataNodesForm = this.formGroup.get('dataNodes') as FormArray;
    dataNodesForm.clear();
    each(rowData, (item, index) => {
      dataNodesForm.push(
        this.fb.group({
          parentUuid: new FormControl(this.setNode(index), {
            validators: [this.baseUtilService.VALID.required()]
          })
        })
      );
    });
  }

  initNodeOptions(maxNumber: number, location: string) {
    const chineseNums = ['零', '一', '两', '三', '四', '五'];
    const nodeOpts = chineseNums.slice(0, maxNumber);
    this.nodeOptions = map(nodeOpts, (item, index) => {
      return {
        key: index + 1,
        label: this.i18n.get(
          'protection_instance_node_option_label',
          this.i18n.isEn ? [index] : [item]
        ),
        value: index + 1,
        isLeaf: true
      };
    });
    if (location === RestoreV2LocationType.ORIGIN) {
      this.originalDRNode = this.nodeOptions;
    }
  }

  getClusterOptions(labelParams?: any) {
    const conditions = {
      subType: DataMap.Resource_Type.tdsqlCluster.value
    };
    extendParams(conditions, labelParams);
    const extParams = {
      conditions: JSON.stringify(conditions)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      param => this.protectedResourceApiService.ListResources(param),
      res => {
        this.clusterOptions = map(
          reject(res, data => data?.uuid === this.originalEnvId),
          item => {
            return {
              key: item.uuid,
              value: item.uuid,
              label: item.name,
              isLeaf: true,
              ...item
            };
          }
        );
      }
    );
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        pluginType: `${DataMap.Resource_Type.tdsqlCluster.value}Plugin`
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.clientManagerApiService.queryAgentListInfoUsingGET(params),
      resource => {
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            item.extendInfo.scenario === DataMap.proxyHostType.external.value
        );
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}(${item.endpoint})`,
            isLeaf: true,
            ipList: get(item.extendInfo, 'agentIpList', []).split(',')
          });
        });
        this.proxyOptions = [...hostArray];
        this.cacheProxyOptions = [...hostArray];
      }
    );
  }

  capacityCalculateLabel(rawData) {
    return map(rawData, item =>
      this.capacityCalculate.transform(item, '1.0-3', 'MB', false, false, 1000)
    );
  }

  getMachineAndHostById(envId: string, location: string) {
    const extParams = {
      envId,
      resourceType: DataMap.Resource_Type.tdsqlCluster.value,
      conditions: JSON.stringify({ queryType: 'resource' })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params =>
        this.protectedEnvironmentApiService.ListEnvironmentResource(params),
      res => {
        if (isEmpty(res)) return;
        this.showAdvanced[location] = true;
        const machineStr = get(first(res), 'extendInfo.machineSpec', '{}');
        const hostStr = get(first(res), 'extendInfo.hosts', '{}');
        const machineData = JSON.parse(machineStr);
        const hostData = JSON.parse(hostStr);
        const machineArr = map(machineData, item => {
          const { machine, cpu, ...rest } = item;
          const formattedLabel = this.capacityCalculateLabel(rest);
          return {
            key: machine,
            value: { ...item },
            isLeaf: true,
            label: this.i18n.get('protection_tdsql_machine_type_label', [
              machine,
              cpu,
              ...formattedLabel
            ])
          };
        });
        this.initNodeOptions(size(hostData), location);
        this.proxyOptions = [...this.cacheProxyOptions]; // 每次处理代理主机之前都恢复一次所有数据
        this.proxyOptions = filter(this.proxyOptions, item => {
          const intersectionArr = intersection(item.ipList, hostData);
          if (!!size(intersectionArr)) {
            return assign(item, { ip: intersectionArr[0] });
          }
        });
        if (location === RestoreV2LocationType.ORIGIN) {
          this.originalMachineOptions = [...machineArr];
          this.originalDataNodes = [...hostData];
        }
        this.machineOptions = [...machineArr];
      },
      err => {
        this.showAdvanced[location] = false;
      }
    );
  }

  getParams() {
    const isOriginal =
      this.formGroup.value.restoreLocation === RestoreV2LocationType.ORIGIN;
    const envId = isOriginal ? this.originalEnvId : this.formGroup.value.target;
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv: envId,
      restoreType: RestoreV2Type.CommonRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject: isOriginal
        ? this.resourceData.uuid
        : find(this.clusterOptions, { uuid: envId })['name'],
      extendInfo: {
        drMode: this.formGroup.value.drMode,
        restoreHosts: JSON.stringify(
          map(this.formGroup.value.dataNodes, item => ({
            ...item,
            ip: find(this.cacheProxyOptions, { uuid: item.parentUuid }).ip
          }))
        )
      }
    };
    if (this.showAdvanced[this.formGroup.value.restoreLocation]) {
      const {
        machine: selectedMachine,
        ...inputParams
      } = this.formGroup.value.machineInfo;
      if (!!selectedMachine) {
        assign(params, {
          extendInfo: {
            ...params.extendInfo,
            extParameter: JSON.stringify({
              machine: selectedMachine.machine,
              ...inputParams
            })
          }
        });
      }
    }
    if (this.rowCopy.backup_type === DataMap.CopyData_Backup_Type.log.value) {
      const properties = JSON.parse(this.rowCopy.properties || '{}');
      assign(params, {
        extendInfo: {
          ...params.extendInfo,
          restoreTimestamp:
            get(this.rowCopy, 'restoreTimeStamp') || get(properties, 'endTime')
        }
      });
    }
    return params;
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
