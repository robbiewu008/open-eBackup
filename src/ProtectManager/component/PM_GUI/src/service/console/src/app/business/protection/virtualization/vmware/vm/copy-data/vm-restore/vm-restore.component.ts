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
  RestoreLocationType,
  RestoreManagerService as RestoreService,
  RestoreType,
  VirtualResourceService,
  VmRestoreOptionType,
  VmwareService,
  LANGUAGE,
  ResourceType,
  Scene,
  ClientManagerApiService,
  Features
} from 'app/shared';
import {
  assign,
  cloneDeep,
  each,
  find,
  includes,
  isBoolean,
  isEmpty,
  isNumber,
  map,
  reject
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-vm-restore',
  templateUrl: './vm-restore.component.html',
  styleUrls: ['./vm-restore.component.less']
})
export class VmRestoreComponent implements OnInit {
  find = find;
  isEmpty = isEmpty;
  unitconst = CAPACITY_UNIT;
  childResType;
  rowCopy;
  // 普通恢复 及时恢复
  restoreType;
  RestoreType = RestoreType;
  showInstanceRestoreTip;
  rowProperties = {
    disk_info: [],
    net_work: []
  };
  dataMap = DataMap;
  formGroup: FormGroup;
  restoreLocationType = RestoreLocationType;

  //  磁盘恢复 虚拟机恢复
  VmRestoreOptionType = VmRestoreOptionType;

  vCenterData = [];
  selectedHost;
  cacheSlectedHost;

  originLocation;
  originName;
  newLocation;
  restoreToNewLocationOnly;
  environmentSubType;
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
  DatastoreType = DatastoreType;
  networkOptions = [];
  datastoreErrorTip = '';
  datastoreNoData = false;
  networkNoData = false;
  notProtectedAllDisk = false;
  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_valid_name_label'),
    invalidSameName: this.i18n.get('common_duplicate_name_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [80])
  };
  isEn = this.i18n.language.toLowerCase() === LANGUAGE.EN;
  proxyHostOptions = [];
  showRdmRecoveryTip = false;
  isSupport = true;

  constructor(
    private fb: FormBuilder,
    private baseUtilService: BaseUtilService,
    private environmentsService: EnvironmentsService,
    private virtualResourceService: VirtualResourceService,
    private restoreService: RestoreService,
    private vmwareService: VmwareService,
    private i18n: I18NService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.initData();
    this.initForm();
    this.getProxyHost();
  }

  getProxyHost(recordsTemp?, startPage?) {
    if (this.restoreType !== RestoreType.CommonRestore) {
      return;
    }
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

  initData() {
    const resourceProperties = JSON.parse(this.rowCopy.resource_properties);
    this.showInstanceRestoreTip =
      this.restoreType === RestoreType.InstanceRestore &&
      resourceProperties.environment_sub_type !==
        DataMap.Resource_Type.vmwareVcenterServer.value;
    this.originName = resourceProperties.name;
    this.originLocation = resourceProperties.path;
    this.rowProperties = JSON.parse(this.rowCopy.properties).vmware_metadata;
    this.notProtectedAllDisk =
      resourceProperties.ext_parameters &&
      isBoolean(resourceProperties.ext_parameters.all_disk) &&
      !resourceProperties.ext_parameters.all_disk;
    const vmDataStore = JSON.parse(this.rowCopy.properties).vmware_metadata
      .vmx_datastore;
    if (this.restoreType === RestoreType.InstanceRestore) {
      this.rowProperties.disk_info = map(this.rowProperties.disk_info, item => {
        return {
          ...item,
          DISKTYPE: 'normal',
          DSNAME: vmDataStore?.name
        };
      });
    }
  }

  initForm() {
    this.restoreToNewLocationOnly =
      this.rowProperties.disk_info[0].DSNAME.startsWith('TemporaryDsForIR') ||
      this.showInstanceRestoreTip ||
      includes(
        [
          DataMap.CopyData_generatedType.replicate.value,
          DataMap.CopyData_generatedType.reverseReplication.value,
          DataMap.CopyData_generatedType.cascadedReplication.value
        ],
        this.rowCopy.generated_by
      ) ||
      this.rowCopy.is_replicated;
    const diskGroupList = [];
    this.originDiskData = cloneDeep(this.rowProperties.disk_info);
    this.rowProperties.disk_info.forEach((item, index) => {
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
        uuid: [item.GUID],
        options: [[]],
        diskType: [item.DISKTYPE || this.diskTypeOps[1].value], // 不返回diskType时就是normal盘
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
      // 修改硬盘类型，
      // 对应的目标数据存储选项改变,
      // 目标数据存储值置空，剩余容量置空，其他剩余容量重算
      group.get('diskType').valueChanges.subscribe(res => {
        if (res === 'rdm') {
          if (this.formGroup.value.storage === DatastoreType.SAME) {
            // 先从已选择的数组中删除该值
            this.sameSelectedRow = this.sameSelectedRow.filter(
              v => group.value.uuid !== v.rowId
            );
            this.computeSameSpace(this.formGroup.value.targetDatastore);
          }
          this.storageDiskTableData.map(v => {
            if (group.value.uuid === v.rowId) {
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
              v => group.value.uuid === v.rowId
            );
            this.sameSelectedRow.push(diskInfo[0]);

            this.computeSameSpace(this.formGroup.value.targetDatastore);
          }
          this.storageDiskTableData.map(v => {
            if (group.value.uuid === v.rowId) {
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

      this.storageDiskTableData.push({
        id: index,
        name: item.NAME,
        slot: item.BUSNUMBER,
        diskCapacity: item.SIZE,
        rowId: item.GUID,
        isRDM: item.DISKTYPE === 'rdm',
        isChange: item.DISKTYPE === 'rdm'
      });
    });

    this.showRdmRecoveryTip = !isEmpty(
      find(this.storageDiskTableData, disk => disk.isRDM)
    );

    const networkGroupList = [];
    this.rowProperties.net_work.forEach(item => {
      networkGroupList.push(
        this.fb.group({
          adapterName: item,
          networkName: new FormControl(
            '',
            this.restoreToNewLocationOnly
              ? {
                  validators: [this.baseUtilService.VALID.required()],
                  updateOn: 'change'
                }
              : null
          )
        })
      );
    });
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreLocationType.ORIGIN),
      location: new FormControl(this.originLocation, {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      name: [],
      deleteOriginalVM: new FormControl(false),
      storage: [DatastoreType.DIFFERENT],
      vmStorage: new FormControl(
        '',
        this.restoreToNewLocationOnly
          ? {
              validators: [this.baseUtilService.VALID.required()],
              updateOn: 'change'
            }
          : null
      ),
      diskStorage: this.fb.array(diskGroupList),
      targetDatastore: [],
      network: this.fb.array(networkGroupList),
      powerOn: [true],
      startupNetworkAdaptor: [false],
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

    this.addNewNameValitors();

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

    this.listenForm();
  }

  listenForm() {
    this.formGroup.get('proxyHost').valueChanges.subscribe(res => {
      if (isEmpty(res)) {
        this.isSupport = true;
        return;
      }
      const params = {
        hostUuidsAndIps: res,
        applicationType: 'VMware',
        scene: Scene.Restore,
        buttonNames: [Features.SnapshotGeneration]
      };
      this.clientManagerApiService
        .queryAgentApplicationUsingPOST({
          AgentCheckSupportParam: params,
          akOperationTips: false
        })
        .subscribe(res => {
          this.isSupport = res?.SnapshotGeneration;
          if (!this.isSupport) {
            this.formGroup.get('isStartupSnapGen').setValue(false);
          }
        });
    });
  }

  // RDM盘不允许恢复到集群，只能恢复到主机
  disableRdmType(event) {
    const controls = (this.formGroup.get('diskStorage') as FormArray).controls;
    each(this.storageDiskTableData, (disk, index) => {
      if (
        event.subType !== DataMap.Resource_Type.hostSystem.value &&
        disk.isRDM
      ) {
        controls[index].get('diskType')?.setValue('normal');
        assign(disk, {
          disableDiskType: true
        });
      } else {
        if (disk.isRDM) {
          controls[index].get('diskType')?.setValue('rdm');
        }
        delete disk.disableDiskType;
      }
    });
  }

  changeLocation(event) {
    this.newLocation = event.path;
    this.environmentSubType = event.environmentSubType;
    this.formGroup.patchValue({ location: event.path });
    this.selectedHost = event.uuid;
    if (this.selectedHost === this.cacheSlectedHost) {
      return;
    }
    this.cacheSlectedHost = this.selectedHost;

    if (event.unselected) {
      this.targetDatastoreOptions = [];
      this.formGroup.patchValue({ targetDatastore: null, vmStorage: null });
      return;
    }
    this.disableRdmType(event);
    this.getStorageTableData();
    this.getNetworkTableData();
  }

  changeVcenter(event) {
    const selectedObj = event.key;
    if (selectedObj === this.selectedVCenter) {
      return;
    }
    this.selectedVCenter = selectedObj;
    this.getRdmStorages();
  }

  listenRestoreLocation() {
    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreLocationType.ORIGIN) {
        this.updateOriginForm();
        this.formGroup.patchValue({ location: this.originLocation });
      } else {
        this.formGroup.patchValue({ location: this.newLocation });
        this.updateNewForm();
      }
    });
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

  getStorageTableData() {
    this.vmwareService
      .listComputeResDatastoreV1ComputeResourcesComputeResUuidDatastoresGet({
        computeResUuid: this.selectedHost
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
            diskStorageControls.controls.forEach(control => {
              const diskDatastoreOptions = cloneDeep(
                this.targetDatastoreOptions
              );
              diskDatastoreOptions.forEach(option => {
                option.diskCapacity = control.value.diskCapacity;
                option.rowId = control.value.uuid;
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
                diskDatastore: null
              });
            });
          }
          this.sameTargetOps = cloneDeep(this.targetDatastoreOptions);
          this.sameTargetOps.forEach(v =>
            assign(v, { diskCapacity: this.allDiskCapacity })
          );
        },
        error: ex => {
          this.datastoreNoData = false;
          this.storageServiceError();
        }
      });
  }

  getRdmStorages() {
    this.vmwareService
      .getRegisterStorages({
        envId: this.selectedVCenter
      })
      .subscribe(res => {
        if (!res.length) {
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
              option.rowId = control.value.uuid;
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
      const rowSelection = selectArr.find(row => row.id === item.rowId);

      if (rowSelection) {
        rowSelection['value'] = item.diskCapacity;
        rowSelection['optionkKey'] = item.key;
      } else {
        // 存储不同数据
        selectArr.push({
          optionkKey: item.key, // 选中的目标存储的key
          value: item.diskCapacity, // 选中的目标存储所在行的硬盘容量
          id: item.rowId // 选中的目标存储所在行的uuid
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
            if (selection.rowIdList.includes(control.value.uuid)) {
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
      v.rowId = group.value.uuid;
    });
    group.get('options').setValue(newOps);

    // 先从已选择的数组中删除该值
    selectArr = selectArr.filter(
      row => row.id !== group.value.diskDatastore?.rowId
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
          rowId: v.rowId
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

  initStorage() {
    this.selectedDiskRow = [];
    this.selectedRdmDiskRow = [];
    // 取出所有普通盘数据
    this.sameSelectedRow = cloneDeep(this.storageDiskTableData).filter(
      v => !v.isRDM
    );
    this.targetDatastoreOptions = [];
    this.formGroup.patchValue({ targetDatastore: null, vmStorage: null });
    this.formGroup.get('targetDatastore').markAsPristine();
    this.formGroup.get('targetDatastore').markAsUntouched();
    this.formGroup.get('vmStorage').markAsPristine();
    this.formGroup.get('vmStorage').markAsUntouched();
    this.formGroup.get('diskStorage').markAsPristine();
    this.formGroup.get('diskStorage').markAsUntouched();
  }

  getNetworkTableData() {
    this.vmwareService
      .listComputeResNetworkV1ComputeResourcesComputeResUuidNetworksGet({
        computeResUuid: this.selectedHost
      })
      .subscribe({
        next: res => {
          if (!res.length) {
            this.networkNoData = true;
            this.networkServiceError();
            return;
          }
          this.networkNoData = false;
          this.initNetwork();
          res.forEach(item => {
            this.networkOptions.push({
              key: item.mo_id,
              label: item.name,
              isLeaf: true
            });
          });
        },
        error: ex => {
          this.networkNoData = false;
          this.networkServiceError();
        }
      });
  }

  networkServiceError() {
    this.initNetwork();
  }

  initNetwork() {
    this.networkOptions = [];
    (this.formGroup.get('network') as FormArray).controls.forEach(item => {
      item.patchValue({
        networkName: null
      });
    });
    this.formGroup.get('network').markAsPristine();
    this.formGroup.get('network').markAsUntouched();
  }

  updateOriginForm() {
    this.formGroup.get('targetDatastore').clearValidators();
    this.formGroup.get('targetDatastore').updateValueAndValidity();
    this.removeVmDiskStorageValitors();
    this.removeNetworkValidators();
  }

  updateNewForm() {
    this.addVmDiskStorageValidators();
    this.addNetworkValidators();
  }

  addNewNameValitors() {
    const nameControl = this.formGroup.get('name');
    if (nameControl.validator) {
      return;
    }
    nameControl.setValidators([
      this.baseUtilService.VALID.required(),
      this.baseUtilService.VALID.maxLength(80)
    ]);
    nameControl.updateValueAndValidity();
  }

  addVmDiskStorageValidators() {
    this.formGroup
      .get('vmStorage')
      .setValidators([this.baseUtilService.VALID.required()]);
    this.formGroup.get('vmStorage').updateValueAndValidity();

    const controls = (this.formGroup.get('diskStorage') as FormArray).controls;
    controls.forEach(control => {
      control
        .get('diskDatastore')
        .setValidators([this.baseUtilService.VALID.required()]);
      control.get('diskDatastore').updateValueAndValidity();
    });
  }

  removeVmDiskStorageValitors() {
    this.formGroup.get('vmStorage').clearValidators();
    this.formGroup.get('vmStorage').updateValueAndValidity();

    const controls = (this.formGroup.get('diskStorage') as FormArray).controls;
    controls.forEach(control => {
      control.get('diskDatastore').clearValidators();
      control.get('diskDatastore').updateValueAndValidity();
    });
  }

  addNetworkValidators() {
    const controls = (this.formGroup.get('network') as FormArray).controls;
    controls.forEach(control => {
      control
        .get('networkName')
        .setValidators([this.baseUtilService.VALID.required()]);
      control.get('networkName').updateValueAndValidity();
    });
  }

  removeNetworkValidators() {
    const controls = (this.formGroup.get('network') as FormArray).controls;
    controls.forEach(control => {
      control.get('networkName').clearValidators();
      control.get('networkName').updateValueAndValidity();
    });
  }

  getTargetPath() {
    return this.formGroup.value.restoreLocation === RestoreLocationType.ORIGIN
      ? this.originLocation
      : this.formGroup.value.location;
  }

  restore(): Observable<void> {
    let extParameters: any = {
      restore_op_type: VmRestoreOptionType.VM,
      vm_name: this.formGroup.value.name,
      power_on: this.formGroup.value.powerOn + '',
      startup_network_adaptor: this.formGroup.value.startupNetworkAdaptor
    };
    if (this.restoreType === RestoreType.CommonRestore) {
      assign(extParameters, {
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
      });
    }
    if (this.formGroup.value.restoreLocation === RestoreLocationType.ORIGIN) {
      extParameters = {
        ...extParameters,
        isDeleteOriginalVM: this.formGroup.value.deleteOriginalVM + ''
      };
    } else {
      const loacationInfo = [];
      (this.formGroup.get('diskStorage') as FormArray).controls.forEach(
        control => {
          let nameStr = '';
          if (
            this.formGroup.value.storage === DatastoreType.SAME &&
            control.value.diskType === 'normal'
          ) {
            nameStr = this.formGroup.value.targetDatastore.key;
          } else {
            nameStr = control.value.diskDatastore.key;
          }
          loacationInfo.push({
            diskId: control.value.uuid,
            storageName: nameStr,
            diskType: control.value.diskType
          });
        }
      );
      const storageInfo = {
        vmstorage_location:
          this.formGroup.value.vmStorage ||
          this.formGroup.value.targetDatastore.key,
        disk_storage_location: loacationInfo
      };

      const networkInfo = {};
      (this.formGroup.get('network') as FormArray).controls.forEach(item => {
        networkInfo[item.value.adapterName] = item.value.networkName;
      });
      extParameters = {
        ...extParameters,
        config: JSON.stringify({
          storage: storageInfo, // 普通盘磁盘文件信息
          network: networkInfo
        })
      };
    }

    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        copy_id: this.rowCopy.uuid,
        object_type: this.rowCopy.resource_sub_type,
        restore_location: this.formGroup.value.restoreLocation,
        filters: [],
        restore_objects: [],
        restore_type: this.restoreType,
        target: {
          details: [],
          env_id:
            this.formGroup.value.restoreLocation === RestoreLocationType.NEW
              ? this.selectedHost
              : '',
          env_type: this.environmentSubType || 'Host',
          restore_target: this.formGroup.value.location
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
