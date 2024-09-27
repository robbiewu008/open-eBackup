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
import { MessageService } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  LANGUAGE,
  RestoreV2LocationType,
  DatastoreType,
  ResourceType,
  CAPACITY_UNIT
} from 'app/shared';
import {
  AppService,
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  includes,
  isEmpty,
  isNumber,
  isString,
  isUndefined,
  map,
  map as _map,
  size,
  uniqBy,
  set
} from 'lodash';
import { forkJoin, Observable, Observer, of, Subject } from 'rxjs';
import { mergeMap } from 'rxjs/operators';

@Component({
  selector: 'aui-fusion-compute-file-restore',
  templateUrl: './fusion-compute-file-restore.component.html',
  styleUrls: ['./fusion-compute-file-restore.component.less']
})
export class FusionComputeFileRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  unitconst = CAPACITY_UNIT;
  disk$ = new Subject<boolean>();
  filterParams = [];
  dataMap = DataMap;
  restoreLocationType = RestoreV2LocationType;
  formGroup: FormGroup;
  resourceData;
  isEn = this.i18n.language.toLowerCase() === LANGUAGE.EN;
  DatastoreType = DatastoreType;
  resourceProperties;
  properties;
  rowProperties;
  storageDiskTableData = [];
  allDiskCapacity = 0;
  selectedDiskRow = [];
  targetDatastoreErrorTip = this.i18n.get(
    'protection_restore_new_datastore_tip_label'
  );

  environmentOptions = [];
  dataStoreOptions = [];
  expandedNodeList = [];
  treeData;
  oldStorePositionOptions = [];
  restoreToNewLocationOnly;
  originName;
  originLocation;
  datastoreNoData = false;
  disabledOrigin = false;

  verifyStatus;
  copyVerifyDisableLabel: string;
  CopyDataVerifyStatus = DataMap.HCSCopyDataVerifyStatus;

  proxyOptions = [];

  constructor(
    public i18n: I18NService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private appService: AppService,
    private messageService: MessageService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.expandedNodeList = [];
    this.treeData = [];
    this.initData();
    this.initForm();
    this.initCopyVerifyDisableLabel();
    this.getProxyOptions();
  }

  initData() {
    this.resourceProperties = JSON.parse(this.rowCopy.resource_properties);
    this.properties = JSON.parse(this.rowCopy.properties);
    // 复制副本、反向复制、级联复制、磁带归档、对象存储归档
    if (
      [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.cloudArchival.value,
        DataMap.CopyData_generatedType.tapeArchival.value,
        DataMap.CopyData_generatedType.reverseReplication.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ].includes(this.rowCopy.generated_by) &&
      isEmpty(this.properties.disk_info)
    ) {
      assign(this.properties, {
        disk_info: this.properties.extendInfo?.disk_info
      });
    }
    this.originName = this.resourceProperties.name;
    this.originLocation = this.resourceProperties.path.replace(
      `/${this.originName}`,
      ''
    );

    if (isString(this.properties?.disk_info)) {
      this.rowProperties = JSON.parse(this.properties?.disk_info || '{}');
    } else {
      this.rowProperties = this.properties?.disk_info;
    }
    this.verifyStatus = this.properties?.verifyStatus;
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
    const diskGroupList = [];
    this.rowProperties.forEach((item, index) => {
      this.allDiskCapacity += +item.quantityGB;
      const group = this.fb.group({
        name: [item.name],
        slot: `${item.pciType}(${item.sequenceNum})`,
        diskCapacity: +item.quantityGB,
        datastoreCapacity: '',
        uuid: item.uuid,
        options: [[]],
        diskDatastore: new FormControl(
          null,
          this.restoreToNewLocationOnly
            ? {
                validators: [this.baseUtilService.VALID.required()],
                updateOn: 'change'
              }
            : null
        )
      });
      diskGroupList.push(group);
      group.get('diskDatastore').valueChanges.subscribe(res => {
        if (res === null) {
          return;
        }
        const rowSelection = this.selectedDiskRow.find(
          row => row.id === res.rowId
        );

        if (rowSelection) {
          rowSelection['value'] = res.diskCapacity;
          rowSelection['optionkKey'] = res.key;
        } else {
          this.selectedDiskRow.push({
            optionkKey: res.key, // 选中的目标存储的key
            value: res.diskCapacity, // 选中的目标存储所在行的硬盘容量
            id: res.rowId // 选中的目标存储所在行的uuid
          });
        }
        const selectedDiskDatastore = [];
        this.selectedDiskRow.forEach(row => {
          const selection = selectedDiskDatastore.find(
            option => option.key === row.optionkKey
          );
          if (selection) {
            selection['value'] += row.value;
            selection['rowIdList'].push(row.id);
          } else {
            selectedDiskDatastore.push({
              key: row.optionkKey,
              value: row.value,
              rowIdList: [row.id]
            });
          }
        });
        if (selectedDiskDatastore.length > 0) {
          const controls = (this.formGroup.get('diskStorage') as FormArray)
            .controls;
          controls.forEach(control => {
            control.value.options.forEach(option => {
              const selection = selectedDiskDatastore.find(
                selectionDatastore => selectionDatastore.key === option.key
              );
              if (selection) {
                const freeSpace = option.cacheFreeSpace - selection.value;
                option.freeSpace = freeSpace < 0 ? 0 : freeSpace;
                if (selection.rowIdList.includes(control.value.uuid)) {
                  option.errorTip =
                    freeSpace < 0 ? this.targetDatastoreErrorTip : '';
                } else {
                  option.errorTip =
                    option.freeSpace < control.value.diskCapacity
                      ? this.targetDatastoreErrorTip
                      : '';
                }
              } else {
                option.freeSpace = option.cacheFreeSpace;
                option.errorTip =
                  option.freeSpace < control.value.diskCapacity
                    ? this.targetDatastoreErrorTip
                    : '';
              }
            });
          });
        }
      });
      this.storageDiskTableData.push({
        id: index,
        name: item.name,
        uuid: item.uuid,
        slot: `${item.pciType}(${item.sequenceNum})`
      });
    });
    this.restoreToNewLocationOnly =
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) || this.rowCopy.is_replicated;

    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      environment: new FormControl(''),
      cluster: new FormControl(
        this.rowCopy.resource_id ? this.rowCopy.resource_id : ''
      ),
      dataStore: new FormControl(''),
      proxyHost: new FormControl([]),
      power_on: new FormControl(false),
      copyVerify: new FormControl(false),
      vm_rename: new FormControl(false),
      vm_name: new FormControl(''),
      rewriteVm: new FormControl(false),
      storage: [DatastoreType.SAME],
      diskStorage: this.fb.array(diskGroupList),
      host: new FormControl([]),
      originPosition: new FormControl({
        value: this.originLocation,
        disabled: true
      })
    });

    // 恢复位置选项变化
    this.listenRestoreLocation();

    // 存储位置选项变化
    this.listenStorage();

    this.formGroup.get('environment').valueChanges.subscribe(res => {
      this.formGroup.get('host').setValue('', { emitEvent: false });
      const diskStorageControls = this.formGroup.get(
        'diskStorage'
      ) as FormArray;
      diskStorageControls.controls.forEach(item => {
        item.patchValue({
          options: [],
          diskDatastore: null
        });
      });
      this.formGroup.get('dataStore').setValue('', { emitEvent: false });
      this.dataStoreOptions = [];
      this.treeData = [];
      this.expandedNodeList = [];
      if (isEmpty(res)) {
        return;
      }
      defer(() => this.getTreeData());
    });

    this.formGroup.get('host').valueChanges.subscribe(res => {
      const diskStorageControls = this.formGroup.get(
        'diskStorage'
      ) as FormArray;
      diskStorageControls.controls.forEach(item => {
        item.patchValue({
          options: [],
          diskDatastore: null
        });
      });
      this.dataStoreOptions = [];
      this.formGroup.get('dataStore').setValue('', { emitEvent: false });
      if (isEmpty(res)) {
        return;
      }
      defer(() => this.getStorageTableData());
    });

    if (
      this.rowCopy?.resource_status === 'NOT_EXIST' ||
      this.restoreToNewLocationOnly
    ) {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
      this.disabledOrigin = true;
    }
  }

  listenRestoreLocation() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (res === RestoreV2LocationType.ORIGIN) {
        this.updateOld();
      } else {
        this.updateNew();
        this.getEnvironment();
      }
      this.formGroup.updateValueAndValidity();
    });

    this.formGroup.get('vm_rename').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('vm_name')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('rewriteVm').setValue(false, { emitEvent: false });
      } else {
        this.formGroup.get('vm_name').clearValidators();
      }
      this.formGroup.get('vm_name').updateValueAndValidity();
    });

    this.formGroup.get('rewriteVm').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup.get('vm_rename').setValue(false, { emitEvent: false });
        this.formGroup.get('vm_name').clearValidators();
        this.formGroup.get('vm_name').updateValueAndValidity();
      }
    });
  }

  listenStorage() {
    this.formGroup.get('storage').valueChanges.subscribe(res => {
      if (res === DatastoreType.DIFFERENT) {
        this.formGroup.get('dataStore').clearValidators();
        this.addVmDiskStorageValidators();
      } else {
        this.removeVmDiskStorageValidators();
        this.formGroup
          .get('dataStore')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('dataStore').updateValueAndValidity();
      this.formGroup.get('dataStore').markAsPristine();
      this.formGroup.get('dataStore').markAsUntouched();
    });
  }

  addVmDiskStorageValidators() {
    const controls = (this.formGroup.get('diskStorage') as FormArray).controls;
    controls.forEach(control => {
      control
        .get('diskDatastore')
        .setValidators([this.baseUtilService.VALID.required()]);
      control.get('diskDatastore').updateValueAndValidity();
    });
  }

  removeVmDiskStorageValidators() {
    const controls = (this.formGroup.get('diskStorage') as FormArray).controls;
    controls.forEach(control => {
      control.get('diskDatastore').clearValidators();
      control.get('diskDatastore').updateValueAndValidity();
    });
  }

  updateOld() {
    this.formGroup.get('environment').clearValidators();
    this.formGroup
      .get('cluster')
      .setValue(this.rowCopy.resource_id ? this.rowCopy.resource_id : '');
    this.formGroup.get('host').clearValidators();
    this.formGroup.get('dataStore').clearValidators();

    this.formGroup.get('environment').updateValueAndValidity();
    this.formGroup.get('host').updateValueAndValidity();
    this.formGroup.get('dataStore').updateValueAndValidity();
  }

  updateNew() {
    this.formGroup
      .get('environment')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('host')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup
      .get('dataStore')
      .setValidators([this.baseUtilService.VALID.required()]);
  }

  getEnvironment(recordsTemp?: any[], startPage?: number) {
    const conditions = {
      subType: [this.childResType || DataMap.Resource_Type.FusionCompute.value],
      type: ResourceType.PLATFORM
    };

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
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        recordsTemp = recordsTemp.filter(
          item =>
            item.linkStatus === DataMap.resource_LinkStatus_Special.normal.value
        );
        this.environmentOptions = _map(recordsTemp, item => {
          return {
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: item.name,
            isLeaf: true
          };
        });
        this.formGroup.updateValueAndValidity();
        return;
      }
      this.getEnvironment(recordsTemp, startPage);
    });
  }

  getTreeData(event?, startPage?) {
    const conditions = {
      rootUuid: this.formGroup.value.environment,
      type: [ResourceType.CLUSTER]
    };

    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify(conditions)
    };

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        if (item.type === 'VM') {
          return;
        }
        const node = {
          path: item.path,
          label: item.name,
          name: item.name,
          contentToggleIcon: this.getResourceIcon(item),
          type: item.type,
          uuid: item.uuid,
          rootUuid: item.rootUuid,
          children: [],
          isLeaf: false,
          moReference: item.extendInfo?.moReference,
          expanded: this.getExpandedIndex(item.uuid) !== -1
        };
        if (node.expanded) {
          this.getExpandedChangeData(CommonConsts.PAGE_START, node);
        }

        if (!find(this.treeData, { uuid: node.uuid })) {
          this.treeData.push(node);
        }
      });
      startPage++;
      if (res.totalCount - startPage * CommonConsts.PAGE_SIZE * 10 > 0) {
        this.getTreeData(null, startPage);
        return;
      }
      this.treeData = [...this.treeData];
    });
  }

  getExpandedIndex(id) {
    return this.expandedNodeList.findIndex(node => node.uuid === id);
  }

  getExpandedChangeData(startPage, event) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      conditions: JSON.stringify({
        parentUuid: event.uuid
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      res.records.forEach(item => {
        if (item.type !== ResourceType.PLATFORM) {
          if (item.type === 'VM') {
            return;
          }
          const rootNode = this.getRootNode(event),
            node = {
              path: item.path,
              label: item.name,
              name: item.name,
              contentToggleIcon: this.getResourceIcon(item),
              type: item.type,
              uuid: item.uuid,
              rootUuid: event.rootUuid,
              children: [],
              isLeaf: item.type !== ResourceType.VM,
              moReference: item.extendInfo?.moReference,
              expanded: this.getExpandedIndex(item.uuid) !== -1
            };
          if (node.expanded) {
            this.getExpandedChangeData(CommonConsts.PAGE_START, node);
          }
          event.children.push(node);
        }
      });
      startPage++;
      if (res.totalCount - startPage * CommonConsts.PAGE_SIZE * 10 > 0) {
        this.getExpandedChangeData(startPage, event);
        return;
      }
      this.treeData = [...this.treeData];
    });
  }

  expandedChange(event) {
    if (event.expanded) {
      this.expandedNodeList.push({
        uuid: event.uuid,
        rootUuid: event.rootUuid
      });
    } else {
      this.expandedNodeList.splice(this.getExpandedIndex(event.uuid), 1);
    }
    if (!event.expanded || event.children.length) {
      return;
    }
    event.children = [];
    this.getExpandedChangeData(CommonConsts.PAGE_START, event);
  }

  getResourceIcon(node) {
    const nodeResource = find(
      DataMap.Resource_Type,
      item => item.value === node.type
    );
    return nodeResource['icon'] + '';
  }

  getRootNode(node) {
    if (!!node.parent) {
      return this.getRootNode(node.parent);
    } else {
      return node;
    }
  }

  getShowData(agentId, envId, resourceIds): Observable<any> {
    const params = {
      agentId,
      envId,
      pageNo: 1,
      pageSize: CommonConsts.PAGE_SIZE * 10,
      resourceIds
    };
    let curData = [];
    return this.appService.ListResourcesDetails(params).pipe(
      mergeMap((response: any) => {
        curData = [of(response)];

        const totalCount = response.totalCount;
        const pageCount = Math.ceil(totalCount / (CommonConsts.PAGE_SIZE * 10));
        for (let i = 2; i <= pageCount; i++) {
          curData.push(
            this.appService.ListResourcesDetails({
              agentId,
              envId,
              pageNo: i,
              pageSize: CommonConsts.PAGE_SIZE * 10,
              resourceIds
            })
          );
        }
        return forkJoin(curData);
      })
    );
  }

  getStorageTableData() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      queryDependency: true,
      conditions: JSON.stringify({
        subType: this.childResType || DataMap.Resource_Type.FusionCompute.value,
        uuid: this.formGroup.value.environment
      })
    };
    this.protectedResourceApiService
      .ListResources(params)
      .subscribe((res: any) => {
        if (res.records?.length) {
          const onlineAgents = res.records[0]?.dependencies?.agents?.filter(
            item =>
              item.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
          );
          if (isEmpty(onlineAgents)) {
            this.datastoreNoData = true;
            this.storageServiceError();
            return;
          }
          const agentsId = onlineAgents[0].uuid;
          this.getShowData(agentsId, this.formGroup.value.environment, [
            this.formGroup.value.host[0]?.uuid
          ]).subscribe({
            next: response => {
              const totalData = [];
              for (const item of response) {
                totalData.push(...item.records);
              }
              if (!totalData.length) {
                this.datastoreNoData = true;
                this.storageServiceError();
                return;
              }
              this.datastoreNoData = false;
              this.initStorage();
              totalData.forEach(item => {
                this.dataStoreOptions.push({
                  key: item.uuid,
                  value: item.uuid,
                  label: item.name,
                  freeSpace: +item.extendInfo?.freeSizeGB,
                  moReference: item?.extendInfo?.datastoreUri,
                  name: item.name,
                  errorTip:
                    +item.extendInfo?.freeSizeGB < this.allDiskCapacity
                      ? this.targetDatastoreErrorTip
                      : '',
                  cacheFreeSpace: +item.extendInfo?.freeSizeGB,
                  datastoreType: item.extendInfo?.type,
                  isLeaf: true
                });
              });
              const diskStorageControls = this.formGroup.get(
                'diskStorage'
              ) as FormArray;
              if (diskStorageControls) {
                diskStorageControls.controls.forEach(control => {
                  const diskDatastoreOptions = cloneDeep(this.dataStoreOptions);
                  diskDatastoreOptions.forEach(option => {
                    option.diskCapacity = control.value.diskCapacity;
                    option.rowId = control.value.uuid;
                    option.errorTip =
                      option.freeSpace < option.diskCapacity
                        ? this.targetDatastoreErrorTip
                        : '';
                  });
                  control.patchValue({
                    options: diskDatastoreOptions,
                    diskDatastore: null
                  });
                });
              }
            },
            error: ex => {
              this.datastoreNoData = true;
              this.storageServiceError();
            }
          });
        } else {
          this.datastoreNoData = true;
          this.storageServiceError();
        }
      });
  }

  initStorage() {
    this.selectedDiskRow = [];
    this.dataStoreOptions = [];
    this.formGroup.get('dataStore').setValue('');
    this.formGroup.get('dataStore').markAsPristine();
    this.formGroup.get('dataStore').markAsUntouched();
    this.formGroup.get('diskStorage').markAsPristine();
    this.formGroup.get('diskStorage').markAsUntouched();
  }

  storageServiceError() {
    this.initStorage();
    const diskStorageControls = this.formGroup.get('diskStorage') as FormArray;
    if (diskStorageControls) {
      diskStorageControls.controls.forEach(item => {
        item.patchValue({
          options: [],
          diskDatastore: null
        });
      });
    }
  }

  getParentResourceType() {
    return this.protectedResourceApiService.ShowResource({
      resourceId: this.resourceProperties.parent_uuid
    });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [
          `${this.childResType ||
            DataMap.Application_Type.FusionCompute.value}Plugin`
        ]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        each(resource, item => {
          const tmp = item.environment;
          if (
            tmp.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value &&
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
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

  isSameDatastoreType(): boolean {
    const selectedDatastore = map(
      this.formGroup.value.diskStorage,
      'diskDatastore'
    );
    // 升级场景，没有该字段，不拦截
    if (
      find(selectedDatastore, item => {
        return isUndefined(item.datastoreType);
      })
    ) {
      return true;
    }
    if (size(uniqBy(selectedDatastore, 'datastoreType')) > 1) {
      return false;
    }
    return true;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN) {
        const params = {
          copyId: this.rowCopy.uuid,
          agents: this.formGroup.value.proxyHost || [],
          targetEnv: this.resourceProperties.root_uuid,
          restoreType: this.restoreType,
          targetLocation: this.formGroup.value.restoreTo,
          targetObject: this.formGroup.value.vm_rename
            ? this.formGroup.value.vm_name
            : this.originName,
          extendInfo: {
            is_power_on: this.formGroup.value.power_on ? 'true' : 'false',
            is_in_place: 'true',
            is_disk_restore: 'false',
            restoreLocation: this.originLocation,
            copyVerify: this.formGroup.value.copyVerify
          }
        };

        this.getParentResourceType().subscribe(res => {
          params['extendInfo']['params'] = JSON.stringify({
            host: {
              name: res?.name,
              moReference: res?.extendInfo?.moReference
            },
            resource: {
              resourcePoolType: res.type === ResourceType.CLUSTER ? 6 : 5,
              moReference: res?.extendInfo?.moReference
            },
            datastore: {
              name: '',
              moReference: ''
            }
          });

          const diskInfos = isString(this.properties?.disk_info)
            ? JSON.parse(this.properties?.disk_info || '{}')
            : this.properties?.disk_info;
          params['subObjects'] = diskInfos.map(item => item.uuid);

          if (
            this.rowCopy.status === DataMap.copydata_validStatus.invalid.value
          ) {
            assign(params.extendInfo, {
              force_recovery: true
            });
          }

          if (this.formGroup.value.rewriteVm) {
            set(params, 'targetObject', this.resourceProperties?.uuid);
            set(params, 'extendInfo.is_disk_restore', 'true');
            set(
              params,
              'extendInfo.restoreLocation',
              this.resourceProperties.path
            );
            set(
              params,
              'extendInfo.location',
              this.resourceProperties?.extendInfo?.location
            );
            delete params['extendInfo']['params'];
          }

          this.restoreV2Service
            .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
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
      } else {
        const targetObj = find(this.environmentOptions, {
          key: this.formGroup.value.environment
        });

        const selectedHost = this.formGroup.value.host[0];
        const params = {
          copyId: this.rowCopy.uuid,
          agents: this.formGroup.value.proxyHost || [],
          targetEnv: targetObj?.uuid,
          restoreType: this.restoreType,
          targetLocation: this.formGroup.value.restoreTo,
          targetObject: this.formGroup.value.vm_rename
            ? this.formGroup.value.vm_name
            : this.originName,
          extendInfo: {
            is_power_on: this.formGroup.value.power_on ? 'true' : 'false',
            is_in_place: 'false',
            is_disk_restore: 'false',
            restoreLocation: selectedHost.path,
            copyVerify: this.formGroup.value.copyVerify
          }
        };

        params['extendInfo']['params'] = JSON.stringify({
          host: {
            name: selectedHost.name,
            moReference: selectedHost.moReference
          },
          resource: {
            resourcePoolType:
              selectedHost.type === ResourceType.CLUSTER ? 6 : 5,
            moReference: selectedHost.moReference
          },
          datastore: {
            name: '',
            moReference: ''
          }
        });

        if (this.formGroup.value.storage === DatastoreType.DIFFERENT) {
          // 不同数据存储类型不允许下发
          if (!this.isSameDatastoreType()) {
            this.messageService.error(
              this.i18n.get('protection_tagert_database_type_error_label'),
              {
                lvMessageKey: 'lvMsg_datastore_type_error',
                lvShowCloseButton: true
              }
            );
            observer.error(null);
            observer.complete();
            return;
          }
          params['subObjects'] = this.formGroup.value.diskStorage.map(
            item => item.uuid
          );
          each(this.formGroup.value.diskStorage, item => {
            params['extendInfo'][item.uuid] = item.diskDatastore.moReference;
          });
        } else {
          const selectedDatastore = find(this.dataStoreOptions, {
            key: this.formGroup.value.dataStore
          });
          const diskInfos = isString(this.properties?.disk_info)
            ? JSON.parse(this.properties?.disk_info || '{}')
            : this.properties?.disk_info;
          each(diskInfos, item => {
            params['extendInfo'][item.uuid] = selectedDatastore.moReference;
          });
        }

        if (
          this.rowCopy.status === DataMap.copydata_validStatus.invalid.value
        ) {
          assign(params.extendInfo, {
            force_recovery: true
          });
        }

        this.restoreV2Service
          .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
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
      }
    });
  }

  getTargetPath() {
    if (this.formGroup.value.restoreTo === this.restoreLocationType.ORIGIN) {
      return this.originLocation;
    }
    const selectEnvironment = find(this.environmentOptions, {
      uuid: this.formGroup.value.environment
    });
    return `${selectEnvironment.label}/${this.formGroup.value.host[0].label}`;
  }
}
