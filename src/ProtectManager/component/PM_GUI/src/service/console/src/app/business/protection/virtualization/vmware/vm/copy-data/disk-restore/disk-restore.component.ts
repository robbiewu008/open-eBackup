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
import { FormArray, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  DatastoreType,
  EnvironmentsService,
  I18NService,
  ResourceType,
  RestoreLocationType,
  RestoreManagerService as RestoreService,
  RestoreType,
  VmRestoreOptionType,
  VmwareService
} from 'app/shared';
import {
  assign,
  cloneDeep,
  find,
  includes,
  isArray,
  isEmpty,
  isNumber,
  map,
  reject
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-disk-restore',
  templateUrl: './disk-restore.component.html',
  styleUrls: ['./disk-restore.component.less']
})
export class DiskRestoreComponent implements OnInit {
  find = find;
  isEmpty = isEmpty;
  unitconst = CAPACITY_UNIT;
  valid$ = new Subject<boolean>();
  RestoreType = RestoreType;
  childResType;
  rowCopy;
  dataMap = DataMap;
  formGroup: FormGroup;
  resourceProperties;
  restoreLocationType = RestoreLocationType;
  VmRestoreOptionType = VmRestoreOptionType;
  DatastoreType = DatastoreType;
  selectedHost;
  selectedVm;
  cacheHostUuid;
  isBatch = false;

  allDiskDatas = [];
  selectedDiskDatas = [];
  activeIndex = 'selected';
  sameFreeSpace = null;
  diskTypeOps = [
    {
      label: this.i18n.get('protection_vmware_restore_rdm_type_label'),
      value: 'rdm',
      isLeaf: true
    },
    {
      label: this.i18n.get('protection_vmware_restore_normal_type_label'),
      value: 'normal',
      isLeaf: true
    }
  ];
  originDiskData = [];
  rdmTargetDatastoreOps = [];
  sameTargetOps = [];
  selectedVCenter;

  targetDatastoreOptions = [];
  datastoreNoData = false;
  newLocation;
  restoreToNewLocationOnly;

  pageSizeOptions = CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS;
  pageSize = CommonConsts.PAGE_SIZE_SMALL;
  storageDiskTableData = [];
  allDiskCapacity = 0;
  allRdmDiskCapacity = 0;
  allNormalDiskCapacity = 0;
  targetDatastoreErrorTip = this.i18n.get(
    'protection_restore_new_datastore_tip_label'
  );
  selectedDiskRow = [];
  selectedRdmDiskRow = [];
  sameSelectedRow = [];
  proxyHostOptions = [];

  constructor(
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService,
    private restoreService: RestoreService,
    private vmwareService: VmwareService,
    private i18n: I18NService,
    private environmentsService: EnvironmentsService
  ) {}

  ngOnInit() {
    this.initDiskTable();
    this.initForm();
    this.getProxyHost();
  }

  getProxyHost(recordsTemp?, startPage?) {
    const conditions = {
      type: ResourceType.HOST,
      sub_type: DataMap.Resource_Type.VMBackupAgent.value
    };
    this.environmentsService
      .queryResourcesV1EnvironmentsGet({
        pageSize: CommonConsts.PAGE_SIZE_MAX,
        pageNo: startPage || CommonConsts.PAGE_START,
        conditions: JSON.stringify(conditions)
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.items];
        if (
          startPage === Math.ceil(res.total / CommonConsts.PAGE_SIZE_MAX) ||
          !res.total
        ) {
          recordsTemp = reject(
            recordsTemp,
            item =>
              item.link_status !== DataMap.resource_LinkStatus.normal.value
          );
          this.proxyHostOptions = map(recordsTemp, item => {
            return assign(item, {
              key: item.uuid,
              value: item.uuid,
              label: !isEmpty(item.endpoint)
                ? `${item.name}(${item.endpoint})`
                : item.name,
              isLeaf: true
            });
          });
          return;
        }
        this.getProxyHost(recordsTemp, startPage);
      });
  }

  initDiskTable() {
    this.resourceProperties = JSON.parse(this.rowCopy.resource_properties);
    this.allDiskDatas = cloneDeep(
      JSON.parse(this.rowCopy.properties).vmware_metadata.disk_info
    );
    this.isBatch = isArray(this.rowCopy.selection);
    this.selectedDiskDatas = this.isBatch
      ? cloneDeep(this.rowCopy.selection)
      : [cloneDeep(this.rowCopy.selection)];
  }

  selectionChange() {
    this.formGroup.patchValue({
      selectDisk: this.selectedDiskDatas.length ? this.selectedDiskDatas : null
    });
    this.storageDiskTableData = [];
    const controls = (this.formGroup.get('diskStorage') as FormArray).controls;
    const cacheDiskList = [];
    controls.forEach((control, i) => {
      const diskData = this.selectedDiskDatas.filter(
        disk => disk.GUID === control.value.guid
      );
      if (isEmpty(diskData)) {
        cacheDiskList.push({
          guid: control.value.guid,
          size: control.value.diskCapacity
        });
      }
    });
    cacheDiskList.forEach(disk => {
      (this.formGroup.get('diskStorage') as FormArray).removeAt(
        controls.findIndex(control => control.value.guid === disk.guid)
      );
      this.selectedDiskRow.splice(
        this.selectedDiskRow.findIndex(row => row.guid === disk.guid),
        1
      );
      this.allDiskCapacity -= disk.diskCapacity;
    });
    this.selectedDiskDatas.forEach(disk => {
      const controlData = controls.find(
        control => control.value.guid === disk.GUID
      );
      if (isEmpty(controlData)) {
        this.initDiskFormGroup(disk, controls, true);
      }
    });

    controls.forEach((control, index) => {
      this.storageDiskTableData.push({
        id: index,
        guid: control.value.guid,
        name: control.value.name,
        slot: control.value.slot
      });
      control.updateValueAndValidity();
      if (this.targetDatastoreOptions.length) {
        this.targetDatastoreOptions.forEach(option => {
          option.errorTip =
            option.free_space < this.allDiskCapacity
              ? this.targetDatastoreErrorTip
              : '';
        });
        this.getDiskDatastoreOptions(control, index);
      }
    });

    (this.formGroup.get('diskStorage') as FormArray).updateValueAndValidity();
  }

  initDiskFormGroup(item, diskGroupList, isDiskChange?) {
    this.allDiskCapacity += +item.SIZE;
    if (item.DISKTYPE === 'rdm') {
      this.allRdmDiskCapacity += +item.SIZE;
    } else {
      this.allNormalDiskCapacity += +item.SIZE;
    }
    const group = this.fb.group({
      name: [item.NAME],
      slot: [item.BUSNUMBER],
      diskCapacity: [+item.SIZE],
      datastoreCapacity: '',
      guid: [item.GUID],
      options: [[]],
      diskType: [item.DISKTYPE || 'normal'],
      diskDatastore: new FormControl(
        null,
        this.restoreToNewLocationOnly ||
        (isDiskChange &&
          this.formGroup.value.restoreLocation === RestoreLocationType.NEW &&
          this.formGroup.value.storage === DatastoreType.DIFFERENT)
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
      // 查询rdm剩余容量
      if (group.value.diskType === 'rdm') {
        this.computeSpace(this.selectedRdmDiskRow, res);
      } else {
        if (this.formGroup.value.storage === DatastoreType.SAME) {
          this.computeSameSpace(res);
        } else {
          this.computeSpace(this.selectedDiskRow, res);
        }
      }
    });

    // 修改硬盘类型，对应的目标数据存储选项改变,目标数据存储值置空，剩余容量置空，其他剩余容量重算
    group.get('diskType').valueChanges.subscribe(res => {
      if (res === 'rdm') {
        if (this.formGroup.value.storage === DatastoreType.SAME) {
          // 先从已选择的数组中删除该值
          this.sameSelectedRow = this.sameSelectedRow.filter(
            v => group.value.guid !== v.guid
          ); //
          this.computeSameSpace(this.formGroup.value.targetDatastore);
        }
        this.storageDiskTableData.map(v => {
          if (group.value.guid === v.guid) {
            v.isChange = true;
          }
        });
        this.changeDiskType(
          this.rdmTargetDatastoreOps,
          this.selectedDiskRow,
          group
        );
      } else {
        if (this.formGroup.value.storage === DatastoreType.SAME) {
          // 找到当前磁盘数据塞进普通盘数组中
          const diskInfo = this.storageDiskTableData.filter(
            v => group.value.guid === v.guid
          );
          this.sameSelectedRow.push(diskInfo[0]);

          this.computeSameSpace(this.formGroup.value.targetDatastore);
        }
        this.storageDiskTableData.map(v => {
          if (group.value.guid === v.guid) {
            v.isChange = false;
          }
        });
        this.changeDiskType(
          this.targetDatastoreOptions,
          this.selectedRdmDiskRow,
          group
        );
      }
      group.get('diskDatastore').setValue(null);
    });
  }

  initForm() {
    this.restoreToNewLocationOnly =
      this.allDiskDatas[0].DSNAME.startsWith('TemporaryDsForIR') ||
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.reverseReplication.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy?.generated_by
      );
    const diskGroupList = [];
    this.selectedDiskDatas.forEach((item, index) => {
      this.initDiskFormGroup(item, diskGroupList);
      this.storageDiskTableData.push({
        id: index,
        guid: item.GUID,
        name: item.NAME,
        slot: item.BUSNUMBER,
        diskCapacity: item.SIZE,
        isRDM: item.DISKTYPE === 'rdm',
        isChange: item.DISKTYPE === 'rdm'
      });
    });

    this.formGroup = this.fb.group({
      selectDisk: new FormControl(this.selectedDiskDatas, {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      restoreLocation: new FormControl(RestoreLocationType.ORIGIN),
      location: new FormControl(this.resourceProperties.path, {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      storage: [DatastoreType.DIFFERENT],
      diskStorage: this.fb.array(diskGroupList),
      targetDatastore: [],
      isForceNBDSsl: new FormControl(true),
      isStartupSnapGen: new FormControl(true),
      proxyHost: new FormControl([])
    });

    if (this.restoreToNewLocationOnly) {
      this.formGroup.patchValue({
        restoreLocation: RestoreLocationType.NEW,
        location: ''
      });
    }

    // 恢复位置选项变化
    this.listenRestoreLocation();

    // 存储位置选项变化
    this.listenStorage();

    // 改变相同数据下的目标数据存储
    this.formGroup.get('targetDatastore').valueChanges.subscribe(res => {
      if (!res) return;
      if (this.formGroup.value.storage === DatastoreType.SAME) {
        this.computeSameSpace(res);
      }
    });
  }

  changeLocation(event) {
    this.newLocation = event.path;
    this.formGroup.patchValue({ location: event.path });
    this.selectedVm = event.uuid;
    this.selectedHost = event.parent ? event.parent.uuid : '';
    if (
      event.parent &&
      event.subType === DataMap.Resource_Type.virtualMachine.value &&
      event.parent.subType ===
        DataMap.Resource_Type.clusterComputeResource.value
    ) {
      this.vmwareService
        .getVmInfoV1VirtualMachinesVmUuidGet({ vmUuid: this.selectedVm })
        .subscribe({
          next: res => {
            this.datastoreNoData = false;
            if (
              res &&
              res.runtime &&
              res.runtime.host &&
              res.runtime.host.uuid
            ) {
              this.getStorageTableData(res.runtime.host.uuid);
            } else {
              this.cacheHostUuid = this.selectedHost;
              this.storageServiceError();
            }
          },
          error: ex => {
            this.cacheHostUuid = this.selectedHost;
            this.datastoreNoData = false;
            this.storageServiceError();
          }
        });
    } else {
      if (this.selectedHost) {
        this.getStorageTableData(this.selectedHost);
      }
    }
  }

  changeVcenter(event) {
    const selectedObj = event.key;
    if (selectedObj === this.selectedVCenter) {
      return;
    }
    this.selectedVCenter = selectedObj;
    this.getRdmStorages();
  }

  storageServiceError() {
    this.initStorage();
    const diskStorageControls = this.formGroup.get('diskStorage') as FormArray;
    if (diskStorageControls) {
      diskStorageControls.controls.forEach(item => {
        item.patchValue({
          options: item.value.diskType === 'normal' ? [] : item.value.options,
          diskDatastore: null
        });
      });
    }
  }

  initStorage() {
    this.selectedDiskRow = [];
    this.targetDatastoreOptions = [];
    // 取出所有普通盘数据
    this.sameSelectedRow = cloneDeep(this.storageDiskTableData).filter(
      v => !v.isRDM
    );
    this.formGroup.patchValue({ targetDatastore: null });
    this.formGroup.get('targetDatastore').markAsPristine();
    this.formGroup.get('targetDatastore').markAsUntouched();
    this.formGroup.get('diskStorage').markAsPristine();
    this.formGroup.get('diskStorage').markAsUntouched();
  }

  listenRestoreLocation() {
    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreLocationType.ORIGIN) {
        this.updateOriginForm();
        this.formGroup.patchValue({ location: this.resourceProperties.path });
      } else {
        this.formGroup.patchValue({ location: this.newLocation });
        this.updateNewForm();
      }
    });
  }

  updateOriginForm() {
    this.formGroup.get('targetDatastore').clearValidators();
    this.formGroup.get('targetDatastore').updateValueAndValidity();
    this.removeVmDiskStorageValitors();
  }

  updateNewForm() {
    this.addVmDiskStorageValidators();
  }

  listenStorage() {
    this.formGroup.get('storage').valueChanges.subscribe(res => {
      const controls = (this.formGroup.get('diskStorage') as FormArray)
        .controls;
      controls.forEach(control => {
        control.patchValue({ diskDatastore: null });
      });
      this.selectedDiskRow = [];
      this.selectedRdmDiskRow = [];
      if (res === DatastoreType.DIFFERENT) {
        this.formGroup.get('targetDatastore').clearValidators();
        this.formGroup.get('targetDatastore').updateValueAndValidity();
        this.addVmDiskStorageValidators();
      } else {
        this.removeVmDiskStorageValitors();
        this.formGroup
          .get('targetDatastore')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('targetDatastore').updateValueAndValidity();
      }
    });
  }

  getStorageTableData(uuid) {
    if (uuid === this.cacheHostUuid) {
      return;
    }
    this.cacheHostUuid = uuid;
    this.vmwareService
      .listComputeResDatastoreV1ComputeResourcesComputeResUuidDatastoresGet({
        computeResUuid: uuid
      })
      .subscribe({
        next: res => {
          if (!res.length) {
            this.datastoreNoData = true;
            this.storageServiceError();
            return;
          }
          this.datastoreNoData = false;
          this.initStorage();
          res.forEach(item => {
            this.targetDatastoreOptions.push({
              key: item.mo_id,
              uuid: item.uuid,
              label: item.name,
              freeSpace: item.free_space,
              errorTip:
                item.free_space < this.allDiskCapacity
                  ? this.targetDatastoreErrorTip
                  : '',
              cacheFreeSpace: item.free_space,
              isLeaf: true
            });
          });
          const diskStorageControls = this.formGroup.get(
            'diskStorage'
          ) as FormArray;
          if (diskStorageControls) {
            diskStorageControls.controls.forEach((control, index) => {
              this.getDiskDatastoreOptions(control, index);
            });
            diskStorageControls.updateValueAndValidity();
          }
        },
        error: ex => {
          this.datastoreNoData = false;
          this.storageServiceError();
        }
      });
  }

  clearDiskStorage() {
    const diskStorageControls = this.formGroup.get('diskStorage') as FormArray;
    if (diskStorageControls) {
      diskStorageControls.controls.forEach(control => {
        control.patchValue({
          options: [],
          diskDatastore: ''
        });
      });
    }
  }

  getRdmStorages() {
    this.vmwareService
      .getRegisterStorages({
        envId: this.selectedVCenter
      })
      .subscribe(res => {
        if (!res.length) {
          // 清空
          this.clearDiskStorage();
          return;
        }

        res.forEach(item => {
          const rdmStorageOp = {
            key: item.ip,
            label: item.ip,
            freeSpace: 0,
            errorTip: '',
            isLeaf: true
          };
          this.rdmTargetDatastoreOps.push(rdmStorageOp);
          this.getRdmFreeSpace(rdmStorageOp);
        });
      });
  }

  getRdmFreeSpace(item?) {
    this.vmwareService
      .getFreeEffectiveCapacity({
        storageIp: item.label,
        envId: this.selectedVCenter
      })
      .subscribe(res => {
        // 获取当前目标数据存储的剩余容量
        const rdmFreeSpace = Number(res);
        if (item.key) {
          item.freeSpace = rdmFreeSpace;
          item.cacheFreeSpace = rdmFreeSpace;
          item.errorTip =
            rdmFreeSpace < this.allRdmDiskCapacity
              ? this.targetDatastoreErrorTip
              : '';
        }

        const diskStorageControls = this.formGroup.get(
          'diskStorage'
        ) as FormArray;
        if (diskStorageControls) {
          diskStorageControls.controls.forEach(control => {
            const rdmDatastoreOptions = cloneDeep(this.rdmTargetDatastoreOps);
            rdmDatastoreOptions.forEach(option => {
              option.diskCapacity = control.value.diskCapacity;
              option.guid = control.value.guid;
            });
            control.patchValue({
              options:
                control.value.diskType === 'rdm'
                  ? rdmDatastoreOptions
                  : control.value.options,
              diskDatastore: null
            });
          });
        }
      });
  }

  /* 可计算：
  不同数据--普通盘/rdm盘剩余容量
  相同数据--rdm盘剩余容量
  */
  computeSpace(selectArr, item) {
    if (!isEmpty(item)) {
      // 记录当前选择的目标数据存储是否和之前选中的一致，若一致取出来
      const rowSelection = selectArr.find(row => row.id === item.guid);

      if (rowSelection) {
        rowSelection['value'] = item.diskCapacity;
        rowSelection['optionkKey'] = item.key;
      } else {
        // 存储不同数据
        selectArr.push({
          optionkKey: item.key, // 选中的目标存储的key
          value: item.diskCapacity, // 选中的目标存储所在行的硬盘容量
          id: item.guid // 选中的目标存储所在行的uuid
        });
      }
    }

    // 统计所有已选硬盘容量值
    const selectedDiskDatastore = [];
    selectArr.forEach(row => {
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
    // 计算剩余容量--减去硬盘容量
    if (selectedDiskDatastore.length > 0) {
      const controls = (this.formGroup.get('diskStorage') as FormArray)
        .controls;
      controls.forEach(control => {
        control.value.options.forEach(option => {
          const selectCapacity = option.cacheFreeSpace;
          const selection = selectedDiskDatastore.find(
            selectionDatastore => selectionDatastore.key === option.key
          );
          if (selection) {
            const freeSpace = selectCapacity - selection.value;
            option.freeSpace = freeSpace < 0 ? 0 : freeSpace;
            if (selection.rowIdList.includes(control.value.guid)) {
              option.errorTip =
                freeSpace < 0 ? this.targetDatastoreErrorTip : '';
            } else {
              option.errorTip =
                option.freeSpace < control.value.diskCapacity
                  ? this.targetDatastoreErrorTip
                  : '';
            }
          }
        });
      });
    }
  }
  changeDiskType(ops, selectArr, group) {
    const newOps = cloneDeep(ops);
    newOps.forEach(v => {
      v.diskCapacity = group.value.diskCapacity;
      v.guid = group.value.guid;
    });
    group.get('options').setValue(newOps);

    // 先从已选择的数组中删除该值
    selectArr = selectArr.filter(
      row => row.id !== group.value.diskDatastore?.guid
    );
    this.computeSpace(selectArr, null);
  }

  // 计算相同数据--普通盘剩余容量
  computeSameSpace(targetData) {
    if (targetData) {
      // 计算本就是普通盘的剩余容量
      this.sameSelectedRow.forEach(v => {
        assign(targetData, {
          diskCapacity: v.diskCapacity,
          guid: v.guid
        });
      });
      let sumVal = 0;
      this.sameSelectedRow.forEach(v => {
        sumVal += Number(v.diskCapacity);
      });
      let freeSpace = 0;
      freeSpace = targetData.cacheFreeSpace - sumVal;
      this.sameFreeSpace = freeSpace < 0 ? 0 : freeSpace;
    }
  }

  getDiskDatastoreOptions(control, index) {
    const diskDatastoreOptions = cloneDeep(this.targetDatastoreOptions);
    diskDatastoreOptions.forEach(option => {
      this.getDiskDatastoreFreeSpace(control, option);
      option.diskCapacity = control.value.diskCapacity;
      option.rowId = index;
      option.guid = control.value.guid;
      option.errorTip =
        option.freeSpace < option.diskCapacity
          ? this.targetDatastoreErrorTip
          : '';
    });
    control.patchValue({
      options:
        control.value.diskType === 'normal'
          ? diskDatastoreOptions
          : control.value.options,
      diskDatastore:
        control.value.diskType === 'normal' ? '' : control.value.diskDatastore
    });
  }

  getDiskDatastoreFreeSpace(control, option) {
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
    const selection = selectedDiskDatastore.find(
      selectionDatastore => selectionDatastore.key === option.key
    );
    if (selection) {
      const freeSpace = option.cacheFreeSpace - selection.value;
      option.freeSpace = freeSpace < 0 ? 0 : freeSpace;
      if (
        control.value.diskDatastore &&
        option.key === control.value.diskDatastore.key
      ) {
        control.value.diskDatastore.freeSpace = option.freeSpace;
      }
      if (selection.rowIdList.includes(control.value.id)) {
        option.errorTip = freeSpace < 0 ? this.targetDatastoreErrorTip : '';
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
  }

  addVmDiskStorageValidators() {
    const controls = (this.formGroup.get('diskStorage') as FormArray).controls;
    controls.forEach(control => {
      control
        .get('diskDatastore')
        .setValidators([this.baseUtilService.VALID.required()]);
      control.get('diskDatastore').updateValueAndValidity();
    });
    (this.formGroup.get('diskStorage') as FormArray).updateValueAndValidity();
  }

  removeVmDiskStorageValitors() {
    const controls = (this.formGroup.get('diskStorage') as FormArray).controls;
    controls.forEach(control => {
      control.get('diskDatastore').clearValidators();
      control.get('diskDatastore').updateValueAndValidity();
    });
    (this.formGroup.get('diskStorage') as FormArray).updateValueAndValidity();
  }

  restore(): Observable<void> {
    let extParameters: any = {
      restore_op_type: VmRestoreOptionType.DISK,
      startup_network_adaptor: false,
      isForceNBDSsl: this.formGroup.value.isForceNBDSsl,
      isStartupSnapGen: this.formGroup.value.isStartupSnapGen,
      host_list: JSON.stringify(
        map(this.formGroup.value.proxyHost, item => {
          const host = find(this.proxyHostOptions, { value: item });
          return {
            proxy_id: item || '',
            host: host?.endpoint || '',
            name: host?.name || '',
            port: host?.port || ''
          };
        })
      )
    };
    const diskInfo = [];
    if (this.formGroup.value.restoreLocation === RestoreLocationType.NEW) {
      (this.formGroup.get('diskStorage') as FormArray).controls.forEach(
        control => {
          let nameStr = '';
          if (
            this.formGroup.value.storage === DatastoreType.SAME &&
            control?.value.diskType === 'normal'
          ) {
            nameStr = this.formGroup.value.targetDatastore.key;
          } else {
            nameStr = control?.value.diskDatastore.key;
          }
          diskInfo.push({
            diskId: control?.value.guid,
            storageName: nameStr,
            diskType: control?.value.diskType
          });
        }
      );
    } else {
      this.selectedDiskDatas.forEach(disk => {
        diskInfo.push({
          diskId: disk.GUID,
          storageName: disk.DSNAME,
          diskType: disk.DISKTYPE
        });
      });
    }
    extParameters = {
      ...extParameters,
      config: JSON.stringify({
        storage: {
          disk_storage_location: diskInfo
        }
      })
    };
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        copy_id: this.rowCopy.uuid,
        object_type: this.rowCopy.resource_sub_type,
        restore_location: this.formGroup.value.restoreLocation,
        filters: [],
        restore_objects: [],
        restore_type: RestoreType.CommonRestore,
        target: {
          details: [
            {
              src_id: [this.resourceProperties.uuid],
              target_id:
                this.formGroup.value.restoreLocation === RestoreLocationType.NEW
                  ? this.selectedVm
                  : this.resourceProperties.uuid,
              target_type: DataMap.Resource_Type.virtualMachine.value
            }
          ],
          env_id:
            this.formGroup.value.restoreLocation === RestoreLocationType.NEW
              ? this.cacheHostUuid || this.selectedHost
              : '',
          env_type: 'Host',
          restore_target:
            this.formGroup.value.restoreLocation === RestoreLocationType.NEW
              ? this.formGroup.value.location
              : ''
        },
        source: {
          source_location: this.rowCopy.resource_location,
          source_name: this.rowCopy.resource_name
        },
        ext_parameters: extParameters
      };
      this.restoreService
        .createRestoreV1RestoresPost({ body: params })
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
}
