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
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  OverWriteOption,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import {
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import {
  assign,
  cloneDeep,
  defer,
  each,
  filter,
  find,
  get,
  intersection,
  isEmpty,
  isNumber,
  isString,
  set,
  size,
  startsWith
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-volume-restore',
  templateUrl: './volume-restore.component.html',
  styleUrls: ['./volume-restore.component.less'],
  providers: [CapacityCalculateLabel]
})
export class VolumeRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  options = [];
  volumeOptions = [];
  selectionPort = [];
  unitconst = CAPACITY_UNIT;
  dataMap = DataMap;
  restoreLocationType = RestoreV2LocationType;
  fileReplaceStrategy = OverWriteOption;
  formGroup: FormGroup;
  tableConfig: TableConfig;
  tableData: TableData;
  targetData = [];
  displayData = [];
  resourceData;
  location = this.i18n.get('common_target_host_label');
  disableOriginLocation = false;
  originalLocation = RestoreV2LocationType.ORIGIN;
  newLocation = RestoreV2LocationType.NEW;
  isSystemBackup = false;
  hasSystemVolume = false;

  readonly PAGESIZE = CommonConsts.PAGE_SIZE * 10;

  scriptErrorTip = {
    invalidName: this.i18n.get('common_script_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [8192])
  };

  // 不能直接使用的挂载点的卷
  forbiddenPath = [
    '/',
    '/dev',
    '/boot',
    '/lib',
    '/lib64',
    '/proc',
    '/run',
    '/sys'
  ];

  // 不能使用的包含在该目录下的挂载点的卷
  systemVolumeList = [
    '/bin',
    '/etc',
    '/lib',
    '/lib64',
    '/opt',
    '/root',
    '/sbin',
    '/usr',
    '/var',
    '/home'
  ];

  @ViewChild('capacityTpl', { static: true })
  capacityTpl: TemplateRef<any>;
  @ViewChild('volumeTable', { static: false }) volumeTable: ProTableComponent;

  constructor(
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private dataMapService: DataMapService,
    private restoreV2Service: RestoreApiV2Service,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.disableOriginLocation =
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value;
    const params = JSON.parse(this.rowCopy.resource_properties);
    this.isSystemBackup = params.ext_parameters.system_backup_flag;
    this.getResourceData();
    this.initForm();
    this.initTable();
    this.getHosts();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      originalHost: new FormControl({
        value:
          this.resourceData.environment_name ||
          this.resourceData.environment?.name,
        disabled: true
      }),
      host: new FormControl(
        {
          value: '',
          disabled: true
        },
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      ),
      enable_bare_metal_restore: new FormControl(false),
      restore_non_system_volume: new FormControl(false),
      reboot_system_after_restore: new FormControl(false),
      preScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]
      }),
      postScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]
      }),
      executeScript: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(8192),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxScript, false)
        ]
      })
    });

    this.watch();
    this.disableOkBtn();
    if (this.disableOriginLocation) {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
    } else {
      this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.ORIGIN);
    }
  }

  watch() {
    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.volumeOptions = [];
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('host').disable();
      } else {
        this.formGroup.get('host').enable();
      }
    });

    this.formGroup.get('host').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      defer(() => this.getVolumes(res));
    });

    this.formGroup.statusChanges.subscribe(res => {
      defer(() => this.disableOkBtn());
    });

    this.formGroup
      .get('enable_bare_metal_restore')
      .valueChanges.subscribe(res => {
        if (res) {
          this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW);
          this.disableOriginLocation = true;
        } else {
          this.disableOriginLocation =
            this.rowCopy?.resource_status ===
            DataMap.Resource_Status.notExist.value;
          if (!this.disableOriginLocation) {
            this.formGroup
              .get('restoreTo')
              .setValue(RestoreV2LocationType.ORIGIN);
          }
        }

        each(this.displayData, item => {
          item.disabled = res;
        });
        this.tableData = {
          data: this.displayData,
          total: size(this.displayData)
        };
        defer(() => this.disableOkBtn());
      });
  }

  initTable() {
    this.tableConfig = {
      table: {
        fake: false,
        async: false,
        compareWith: 'name',
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label')
          },
          {
            key: 'type',
            name: this.i18n.get('common_type_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('volumeType')
            },
            filter: {
              type: 'select',
              options: this.dataMapService.toArray('volumeType')
            }
          },
          {
            key: 'size',
            name: this.i18n.get('common_size_label'),
            cellRender: this.capacityTpl
          }
        ],
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true,
          keepRadioLogic: true
        },
        colDisplayControl: false,
        selectionChange: data => {
          this.hasSystemVolume = false;
          this.selectionPort = data;
          each(data, item => {
            let tmp = item.mountPoint.split(',');
            each(tmp, volume => {
              if (
                find(this.systemVolumeList, val => {
                  return startsWith(volume, val) || volume === '/';
                })
              ) {
                this.hasSystemVolume = true;
                if (
                  this.formGroup.value.restoreTo ===
                  RestoreV2LocationType.ORIGIN
                ) {
                  this.formGroup
                    .get('restoreTo')
                    .setValue(RestoreV2LocationType.NEW);
                }
              }
            });
          });
          if (!!this.targetData.length) {
            this.targetData = filter(this.targetData, item => {
              return find(this.selectionPort, tmp => tmp.name === item.name);
            });
          }
          each(this.selectionPort, tmp => {
            if (!find(this.targetData, item => item?.name === tmp.name)) {
              this.targetData.push({
                ...tmp,
                name: tmp.name,
                path: tmp.name,
                volumeOptions: filter(this.volumeOptions, item => {
                  return item.size >= tmp.size;
                })
              });
            }
          });
          this.targetData = [...this.targetData];
          this.disableOkBtn();
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showTotal: true
      }
    };
    const volumeArray = JSON.parse(this.rowCopy.properties).volumeInfoSet;
    each(volumeArray, item => {
      let tmp = item.mountPoint.split(',');
      if (!tmp) {
        item.type = false;
      }
      each(tmp, volume => {
        assign(item, {
          type: !!find(this.systemVolumeList, val => {
            return startsWith(volume, val) || volume === '/';
          })
        });
      });
      this.displayData.push({
        ...item,
        name: item.volumePath
      });
    });
    this.tableData = {
      data: this.displayData,
      total: size(this.displayData)
    };
  }

  search(e) {
    this.displayData = [];
    const volumeArray = JSON.parse(this.rowCopy.properties).volumeInfoSet;
    each(volumeArray, item => {
      if (item.volumePath.toLowerCase().indexOf(e.toLowerCase()) !== -1) {
        let tmp = item.mountPoint.split(',');
        if (!tmp) {
          item.type = false;
        }
        each(tmp, volume => {
          assign(item, {
            type: !!find(this.systemVolumeList, val => {
              return startsWith(volume, val) || volume === '/';
            })
          });
        });
        this.displayData.push({
          ...item,
          name: item.volumePath
        });
      }
    });
    this.tableData = {
      data: this.displayData,
      total: size(this.displayData)
    };
  }

  getResourceData() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
  }

  getHosts(recordsTemp?, startPage?) {
    this.protectedResourceApiService
      .ListResources({
        pageSize: this.PAGESIZE,
        pageNo: startPage || 0,
        conditions: JSON.stringify({
          subType: ['VolumePlugin']
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
          startPage === Math.ceil(res.totalCount / this.PAGESIZE) ||
          res.totalCount === 0
        ) {
          const hostArr = [];
          each(recordsTemp, item => {
            if (
              item.environment?.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
            ) {
              hostArr.push({
                key: item.environment.uuid,
                value: item.environment.uuid,
                label: !isEmpty(item.environment?.endpoint)
                  ? `${item.environment?.name}(${item.environment?.endpoint})`
                  : item.environment?.name,
                os_type: item.environment?.osType,
                parentUuid: item.parentUuid,
                isLeaf: true
              });
            }
          });
          this.options = hostArr;
          return;
        }
        this.getHosts(recordsTemp, startPage);
      });
  }

  getVolumes(res, recordsTemp?, startPage?) {
    if (
      !res ||
      this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
    ) {
      return;
    }

    this.protectedEnvironmentApiService
      .ListEnvironmentResource({
        envId: find(this.options, {
          key: res
        })?.parentUuid,
        pageNo: startPage || CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE * 10,
        parentId: '',
        resourceType: DataMap.Resource_Type.volume.value
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
          startPage ===
            Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
          res.totalCount === 0
        ) {
          const tableData = [];
          each(recordsTemp, item => {
            let tmp = item.extendInfo.volumeMountPoints.split(',');
            const diff = !!intersection(this.forbiddenPath, tmp).length;
            let redFlag = false;
            each(tmp, volume => {
              if (
                find(this.systemVolumeList, val => {
                  return startsWith(volume, val);
                })
              ) {
                redFlag = true;
              }
            });
            if (!diff && !redFlag) {
              tableData.push({
                ...item,
                key: get(item, 'extendInfo.path'),
                value: get(item, 'extendInfo.path'),
                label: get(item, 'extendInfo.path'),
                size: get(item, 'extendInfo.size'),
                volume: get(item, 'extendInfo.path'),
                volumeMountPoints: get(
                  item,
                  'extendInfo.volumeMountPoints',
                  '[]'
                ),
                isLeaf: true
              });
            }
          });
          this.volumeOptions = cloneDeep(tableData);
          this.volumeTable._selectionChange(this.selectionPort);
          return;
        }
        this.getVolumes(recordsTemp, startPage);
      });
  }

  getParams() {
    let targetInfo = [];
    if (this.formGroup.value.enable_bare_metal_restore) {
      const volumeArray = JSON.parse(this.rowCopy.properties).volumeInfoSet;
      each(volumeArray, item => {
        targetInfo.push({
          volumeId: item.uuid,
          dataDstPath: item.volumePath
        });
      });
    } else {
      if (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW) {
        each(this.targetData, item => {
          targetInfo.push({
            volumeId: item.uuid,
            dataDstPath: item.volume
          });
        });
      } else {
        each(this.selectionPort, item => {
          targetInfo.push({
            volumeId: item.uuid,
            dataDstPath: item.name
          });
        });
      }
    }

    let target;
    if (this.formGroup.value.restoreTo === RestoreV2LocationType.NEW) {
      target = filter(this.options, item => {
        return item.value === this.formGroup.value.host;
      });
    }

    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resourceData.environment_uuid ||
            this.resourceData.environment?.uuid
          : this.formGroup.value.host,
      restoreType:
        this.restoreType === RestoreType.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: this.formGroup.value.restoreTo,
      targetObject:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? this.resourceData.uuid
          : target[0].label,
      scripts: {
        preScript: this.formGroup.value.preScript,
        postScript: this.formGroup.value.postScript,
        failPostScript: this.formGroup.value.executeScript
      },
      extendInfo: {
        restoreInfoSet: JSON.stringify(targetInfo),
        enable_bare_metal_restore: this.formGroup.value
          .enable_bare_metal_restore
      }
    };
    if (this.formGroup.value.enable_bare_metal_restore) {
      set(
        params,
        'extendInfo.restore_non_system_volume',
        this.formGroup.value.restore_non_system_volume
      );
      set(
        params,
        'extendInfo.reboot_system_after_restore',
        this.formGroup.value.reboot_system_after_restore
      );
    }
    return params;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      resource:
        this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
          ? {
              name: this.resourceData.name,
              value: this.resourceData.uuid
            }
          : assign(
              {},
              find(this.options, {
                value: this.formGroup.value.host
              }),
              {
                name: find(this.options, {
                  value: this.formGroup.value.host
                })?.label
              }
            ),
      restoreLocation: this.formGroup.value.restoreTo,
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? this.resourceData.environment_name ||
          this.resourceData.environment?.name
      : `${
          find(this.options, item => item.value === this.formGroup.value.host)[
            'label'
          ]
        }`;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.restoreV2Service
        .CreateRestoreTask({
          CreateRestoreTaskRequestBody: this.getParams() as any
        })
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

  volumeChange(e?) {
    each(this.volumeOptions, item => {
      if (find(this.targetData, tmp => tmp.volume && tmp.volume === item.key)) {
        item.disabled = true;
      } else {
        item.disabled = false;
      }
    });
    each(this.targetData, item => {
      item.volumeOptions = this.volumeOptions.filter(tmp => {
        return tmp.size >= item.size;
      });
    });
    let unchosen = filter(this.targetData, item => {
      return !item.volume;
    });
    if (unchosen.length !== 0) {
      this.modal.getInstance().lvOkDisabled = true;
    } else if (this.formGroup.valid && !!this.selectionPort.length) {
      this.modal.getInstance().lvOkDisabled = false;
    }
  }

  disableOkBtn() {
    if (this.formGroup.value.enable_bare_metal_restore) {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    } else {
      if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
        this.modal.getInstance().lvOkDisabled =
          this.formGroup.invalid || !this.selectionPort.length;
      } else if (!this.formGroup.invalid) {
        if (!this.selectionPort.length) {
          this.modal.getInstance().lvOkDisabled = true;
        } else {
          this.volumeChange();
        }
      }
    }
  }
}
