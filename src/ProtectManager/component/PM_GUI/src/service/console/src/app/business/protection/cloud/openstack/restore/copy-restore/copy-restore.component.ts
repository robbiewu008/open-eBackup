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
import { CdkDragDrop, moveItemInArray } from '@angular/cdk/drag-drop';
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CapacityCalculateLabel,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService,
  ResourceType,
  RestoreApiV2Service,
  RestoreV2LocationType
} from 'app/shared';
import { TableConfig, TableData } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  defer,
  difference,
  each,
  every,
  filter,
  find,
  includes,
  isEmpty,
  isEqual,
  isString,
  map,
  size,
  some,
  toNumber
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-copy-restore',
  templateUrl: './copy-restore.component.html',
  styleUrls: ['./copy-restore.component.less'],
  providers: [CapacityCalculateLabel]
})
export class CopyRestoreComponent implements OnInit {
  rowCopy;
  childResType;
  restoreType;

  formGroup: FormGroup;
  restoreToNewLocationOnly = false;
  restoreLocationType = RestoreV2LocationType;
  resource;

  diskTableData: TableData;
  diskTableConfig: TableConfig;

  resourceLocation;
  vmType;
  network;
  originalIP; // 原虚拟机IP，老版本没有vm_ip，默认展示--
  serverTreeData = [];
  vmTypeOptions = [];
  networkOptions = [];
  diskTypeOptions = [];
  proxyOptions = [];
  networkIPMap = new Map(); // 保存networkID的映射
  networkAndIPControlArr: FormArray;
  copyDataVerifyStatus = DataMap.HCSCopyDataVerifyStatus;
  verifyStatus;
  copyVerifyDisableLabel;

  azOptions = [];
  cacheVolumeType = [];

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };

  valid$ = new Subject<boolean>();

  @ViewChild('diskTypeTpl', { static: true }) diskTypeTpl: TemplateRef<any>;

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private restoreV2Service: RestoreApiV2Service,
    private capacityCalculateLabel: CapacityCalculateLabel,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.getResource();
    this.initForm();
    this.initCopyVerifyDisableLabel();
    this.getProxyOptions();
  }

  initCopyVerifyDisableLabel() {
    const properties = JSON.parse(this.rowCopy.properties);
    this.verifyStatus = properties?.verifyStatus;
    if (
      includes([this.copyDataVerifyStatus.noGenerate.value], this.verifyStatus)
    ) {
      this.copyVerifyDisableLabel = this.i18n.get(
        'common_generate_verify_file_disable_label'
      );
    }
    if (
      includes([this.copyDataVerifyStatus.Invalid.value], this.verifyStatus)
    ) {
      this.copyVerifyDisableLabel = this.i18n.get(
        'common_invalid_verify_file_disable_label'
      );
    }
  }

  showTypeWarn() {
    return some(
      this.diskTableData?.data,
      item => !isEmpty(item.diskType) && item.diskType !== item.volumeType
    );
  }

  getResource() {
    this.restoreToNewLocationOnly = includes(
      [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ],
      this.rowCopy.generated_by
    );
    this.resource = JSON.parse(this.rowCopy?.resource_properties || '{}');
    this.resourceLocation = this.resource.path;
    this.originalIP = this.resource.extendInfo?.vm_ip || ''; // 老版本的副本没有vm_ip字段
    this.getVmOptions(this.resource.parent_uuid);
    this.diskTableConfig = {
      table: {
        compareWith: 'id',
        async: false,
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label')
          },
          {
            key: 'type',
            name: this.i18n.get('common_type_label'),
            cellRender: this.diskTypeTpl
          }
        ],
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        showTotal: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
    setTimeout(() => this.getDiskData());
  }

  getDiskData() {
    const properties = JSON.parse(this.rowCopy.properties || '{}');
    let needRestoreDisks = properties?.volList;
    if (isEmpty(needRestoreDisks)) {
      needRestoreDisks = properties.extendInfo?.volList || [];
    }
    each(needRestoreDisks, item => {
      assign(item, {
        volumeType: item.volume_type,
        id: item.uuid,
        size: item.volSizeInBytes / 1024 / 1024 / 1024
      });
    });
    this.diskTableData = {
      data: needRestoreDisks,
      total: size(needRestoreDisks)
    };
  }

  diskTypeChange() {
    this.vaildParams();
  }

  vaildParams() {
    this.valid$.next(
      this.formGroup.valid &&
        every(this.diskTableData?.data, item => !isEmpty(item.diskType))
    );
  }

  getOptions(subType, params, node?) {
    const extParams = {
      conditions: JSON.stringify(params)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        if (subType === ResourceType.OPENSTACK_CONTAINER) {
          this.serverTreeData = map(resource, item => {
            return assign(item, {
              label: item.name,
              disabled: true,
              children: [],
              isLeaf: false,
              expanded: false
            });
          });
        } else {
          each(resource, item => {
            const isOpenStackProject =
              item.subType === DataMap.Resource_Type.openStackProject.value;
            node.children.push(
              assign(item, {
                label: item.name,
                disabled: !isOpenStackProject,
                children: isOpenStackProject ? null : [],
                isLeaf: isOpenStackProject,
                expanded: false
              })
            );
          });
          this.serverTreeData = [...this.serverTreeData];
        }
      }
    );
  }

  getTreeData() {
    if (!isEmpty(this.serverTreeData)) {
      return;
    }
    this.getOptions(ResourceType.OPENSTACK_CONTAINER, {
      type: ResourceType.OpenStack,
      subType: ResourceType.OPENSTACK_CONTAINER
    });
  }

  expandedChange(node) {
    if (!node.expanded || node.children?.length) {
      return;
    }
    node.children = [];
    if (node.subType === ResourceType.OPENSTACK_CONTAINER) {
      this.getOptions(
        ResourceType.OpenStackDomain,
        {
          parentUuid: node.uuid,
          visible: ['1']
        },
        node
      );
    } else {
      this.getOptions(
        ResourceType.OpenStackProject,
        {
          parentUuid: node.uuid
        },
        node
      );
    }
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [
          `${DataMap.globalResourceType.openStackContainer.value}Plugin`
        ]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = resource.filter(
          item =>
            item.environment?.linkStatus ===
            DataMap.resource_LinkStatus_Special.normal.value
        );
        // 过滤1.2.1版本
        resource = this.baseUtilService.rejectAgentsByVersion(
          resource,
          '1.2.1'
        );
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (tmp.extendInfo.scenario === '0') {
            hostArray.push({
              ...tmp,
              key: tmp.uuid,
              value: tmp.uuid,
              label: `${tmp.name}(${tmp.endpoint})`,
              isLeaf: true
            });
          }
        });
        this.proxyOptions = hostArray;
      }
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      targetServer: new FormControl([]),
      az: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      vmType: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      network: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      vmName: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      rewriteVm: new FormControl(false),
      copyVerify: new FormControl(false),
      proxyHost: new FormControl([]),
      specifyIP: new FormControl(true),
      networkAndIPArr: this.fb.array([])
    });
    this.networkAndIPControlArr = this.formGroup.get(
      'networkAndIPArr'
    ) as FormArray;
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      this.clearParams();
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('targetServer').clearValidators();
        this.getVmOptions(this.resource.parent_uuid);
      } else {
        this.getTreeData();
        this.formGroup
          .get('targetServer')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.getVmOptions(this.formGroup.value.targetServer[0]?.uuid);
      }
      this.formGroup.get('targetServer').updateValueAndValidity();
    });

    this.formGroup.get('az').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.diskTypeOptions = filter(
        this.cacheVolumeType,
        item =>
          isEmpty(item['HW:availability_zone']) ||
          item['HW:availability_zone'] === res
      );
      each(this.diskTableData?.data, item => {
        if (find(this.diskTypeOptions, { value: item.volumeType })) {
          assign(item, {
            diskType: item.volumeType
          });
        } else {
          assign(item, {
            diskType: ''
          });
        }
      });
    });

    this.formGroup
      .get('targetServer')
      .valueChanges.subscribe(res => this.getVmOptions(res[0]?.uuid));

    this.formGroup.get('network').valueChanges.subscribe(res => {
      // network变化时收到的是网络id，需要转成对应的ip
      // formGroup.value获取到的是变化前的值，valueChange接收到的是实时的值
      const addedArr = difference(res, this.formGroup.value.network);
      const removedArr = difference(this.formGroup.value.network, res);
      this.createFormIpNode(addedArr, removedArr);
    });

    this.formGroup.statusChanges.subscribe(() => this.vaildParams());

    if (this.restoreToNewLocationOnly) {
      defer(() =>
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW)
      );
    }
  }

  clearParams() {
    this.azOptions = [];
    this.vmTypeOptions = [];
    this.networkOptions = [];
    this.diskTypeOptions = [];
    this.formGroup.get('az').setValue('', { emitEvent: false });
    this.formGroup.get('vmType').setValue('', { emitEvent: false });
    this.formGroup.get('network').setValue([]);
    each(this.diskTableData?.data, item => {
      item.diskType = '';
    });
  }

  getVmOptions(resourceId) {
    if (!resourceId) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({
        resourceId,
        akDoException: false
      })
      .subscribe(res => {
        const az = JSON.parse(res.extendInfo.availabilityZone || '{}');
        const flavor = JSON.parse(res.extendInfo.flavor || '{}');
        const network = JSON.parse(res.extendInfo.network || '{}');
        this.cacheVolumeType = JSON.parse(res.extendInfo.volumeType || '{}');
        this.azOptions = map(az, item => {
          return {
            ...item,
            label: item.zoneName,
            value: item.zoneName,
            isLeaf: true
          };
        });
        this.vmTypeOptions = map(flavor, item => {
          return {
            label: item.name,
            value: item.id,
            isLeaf: true
          };
        });
        this.networkOptions = map(network, item => {
          if (!this.networkIPMap.get(item.id)) {
            // 将整个item保存
            this.networkIPMap.set(item.id, item);
          }
          return assign(item, {
            label: item.name,
            value: item.id,
            isLeaf: true
          });
        });
        this.diskTypeOptions = map(this.cacheVolumeType, item => {
          return assign(item, {
            label: item.name,
            value: item.name,
            isLeaf: true
          });
        });
        let setAz;
        each(this.diskTableData?.data, item => {
          if (find(this.diskTypeOptions, { value: item.volumeType })) {
            assign(item, {
              diskType: item.volumeType
            });
            if (!setAz) {
              setAz = find(this.diskTypeOptions, { value: item.volumeType })[
                'HW:availability_zone'
              ];
            }
          }
        });
        if (
          this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN &&
          !this.restoreToNewLocationOnly
        ) {
          const originNetwork = isString(this.resource.extendInfo?.networks)
            ? this.resource.extendInfo?.networks.split(';')
            : [];
          const setNetwork = map(
            filter(this.networkOptions, item =>
              includes(originNetwork, item.name)
            ),
            'value'
          );
          this.formGroup
            .get('az')
            .setValue(
              includes(map(this.azOptions, 'value'), setAz) ? setAz : '',
              {
                emitEvent: false
              }
            );
          this.formGroup
            .get('vmType')
            .setValue(
              includes(
                map(this.vmTypeOptions, 'value'),
                this.resource.extendInfo?.flavorId
              )
                ? this.resource.extendInfo?.flavorId
                : '',
              {
                emitEvent: false
              }
            );
          this.formGroup.get('network').setValue(setNetwork);
        }
      });
  }

  getNetworkControl(item) {
    return this.fb.group({
      name: new FormControl(item.label), // name为展示的网络名
      value: new FormControl(item.value), // value为选中的网络id，恢复下发时使用
      specifiedIp: new FormControl('', {
        // specifiedIp为指定的ip
        validators: [this.baseUtilService.VALID.ipv4()]
      })
    });
  }

  /**
   * 根据选中的网络构造表单节点
   * @param addedArr 新选中的节点
   * @param removedArr 被移除的节点
   */
  createFormIpNode(addedArr, removedArr) {
    removedArr.forEach(item => {
      const index = this.networkAndIPControlArr.controls.findIndex(ctrl =>
        isEqual(ctrl.get('value').value, item)
      );
      if (index !== -1) {
        this.networkAndIPControlArr.removeAt(index);
      }
    });
    addedArr = addedArr.map(item => this.networkIPMap.get(item));
    addedArr.forEach(item => {
      this.networkAndIPControlArr.push(this.getNetworkControl(item));
    });
  }

  drop(event: CdkDragDrop<string[]>) {
    moveItemInArray(
      this.networkAndIPControlArr.controls,
      event.previousIndex,
      event.currentIndex
    );
    // control更换顺序后，value跟着一起换顺序
    moveItemInArray(
      this.networkAndIPControlArr.value,
      event.previousIndex,
      event.currentIndex
    );
  }

  getParams() {
    const params = {
      copyId: this.rowCopy?.uuid,
      agents: this.formGroup.value.proxyHost,
      restoreType: this.restoreType,
      targetEnv:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resource?.root_uuid
          : this.formGroup.value.targetServer[0]?.rootUuid,
      targetLocation: this.formGroup.value.restoreTo,
      targetObject: JSON.stringify({
        name: this.formGroup.value.vmName,
        parentUuid:
          this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
            ? this.resource?.parent_uuid
            : this.formGroup.value.targetServer[0]?.uuid,
        extendInfo: {
          domainId:
            this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
              ? this.resource?.extendInfo?.domainId
              : this.formGroup.value.targetServer[0]?.extendInfo?.domainId,
          domainName:
            this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
              ? this.resource?.extendInfo?.domainName
              : this.formGroup.value.targetServer[0]?.extendInfo?.domainName
        }
      }),
      subObjects: map(this.diskTableData?.data, item => {
        return JSON.stringify({
          name: item.name,
          uuid: item.id,
          extendInfo: {
            targetVolume: JSON.stringify({
              size: toNumber(item.size),
              volumeTypeName: item.diskType,
              isNewDisk: 'true',
              bootable: item.bootable
            })
          }
        });
      }),
      extendInfo: {
        restoreLevel: 0,
        powerState: '1',
        copyVerify: this.formGroup.value.copyVerify ? 'true' : 'false',
        availabilityZone: JSON.stringify({
          name: this.formGroup.value.az
        }),
        flavor: JSON.stringify({
          id: this.formGroup.value.vmType
        }),
        network: JSON.stringify(
          map(this.networkAndIPControlArr.value, item => {
            return {
              id: item.value,
              ip: item.specifiedIp
            };
          })
        )
      }
    };
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
      assign(params.extendInfo, {
        isDeleteOriginalVM: this.formGroup.value.rewriteVm
      });
    }
    if (this.rowCopy.status === DataMap.copydata_validStatus.invalid.value) {
      assign(params.extendInfo, {
        force_recovery: true
      });
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
