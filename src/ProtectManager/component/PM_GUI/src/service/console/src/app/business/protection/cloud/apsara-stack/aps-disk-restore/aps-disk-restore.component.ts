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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { OptionItem } from '@iux/live';
import {
  AppService,
  BaseUtilService,
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  RestoreApiV2Service,
  RestoreV2LocationType
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  defer,
  differenceBy,
  each,
  every,
  filter,
  find,
  first,
  includes,
  isEmpty,
  isNumber,
  map,
  pick,
  reject,
  size,
  some,
  toNumber
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-aps-disk-restore',
  templateUrl: './aps-disk-restore.component.html',
  styleUrls: ['./aps-disk-restore.component.less'],
  providers: [CapacityCalculateLabel]
})
export class ApsDiskRestoreComponent implements OnInit {
  rowCopy;
  restoreType;

  _isEmpty = isEmpty;
  resourceProperties;
  resourceServer;
  restoreTableConfig: TableConfig;
  restoreTableData: TableData;
  targetTableConfig: TableConfig;
  targetTableData: TableData;

  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  CopyDataVerifyStatus = DataMap.HCSCopyDataVerifyStatus;
  restoreToNewLocationOnly = false;
  serverTreeData = [];
  EXIST_DISK = '1';
  NEW_DISK = '2';
  typeOptions: OptionItem[] = [
    {
      label: this.i18n.get('protection_recovery_exist_disk_label'),
      value: this.EXIST_DISK,
      isLeaf: true
    },
    {
      label: this.i18n.get('protection_recovery_new_disk_label'),
      value: this.NEW_DISK,
      isLeaf: true
    }
  ];
  volumeTypeOptions = [];
  proxyOptions = [];
  verifyStatus;
  copyVerifyDisableLabel;

  targetDisksOptions;
  cacheSelectedDisk = [];

  disk$ = new Subject<boolean>();

  @ViewChild('restoreTable', { static: false }) restoreTable: ProTableComponent;
  @ViewChild('targeTable', { static: false }) targeTable: ProTableComponent;
  @ViewChild('diskDeviceTpl', { static: true }) diskDeviceTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('recoveryTypeTpl', { static: true }) recoveryTypeTpl: TemplateRef<
    any
  >;

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private appService: AppService,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private restoreV2Service: RestoreApiV2Service,
    private capacityCalculateLabel: CapacityCalculateLabel,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.initForm();
    this.initCopyVerifyDisableLabel();
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

  showTypeWarn() {
    return (
      this.formGroup.value?.restoreTo === RestoreV2LocationType.ORIGIN &&
      some(
        this.targetTableData?.data,
        item =>
          item.recoveryType === this.NEW_DISK &&
          !isEmpty(item.targetVolumeType) &&
          item.targetVolumeType !== item.volumeType
      )
    );
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
        resource = resource.filter(
          item =>
            item.environment?.linkStatus ===
            DataMap.resource_LinkStatus_Special.normal.value
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

  initConfig() {
    this.resourceProperties = JSON.parse(
      this.rowCopy?.resource_properties || '{}'
    );
    this.restoreToNewLocationOnly =
      this.rowCopy?.generated_by ===
        DataMap.CopyData_generatedType.cascadedReplication.value ||
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value;
    const properties = JSON.parse(this.rowCopy.properties);
    this.verifyStatus = properties?.verifyStatus;

    const colsLeft: TableCols[] = [
      {
        key: 'nameId',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'mode',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Disk_Mode')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Disk_Mode')
        }
      },
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.sizeTpl
      }
    ];

    const colsRight: TableCols[] = [
      {
        key: 'nameId',
        width: 300,
        name: this.i18n.get('common_restore_disk_name_label')
      },
      {
        key: 'size',
        width: 120,
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.sizeTpl
      },
      {
        key: 'recoveryType',
        width: 180,
        name: this.i18n.get('common_type_label'),
        cellRender: this.recoveryTypeTpl
      },
      {
        key: 'position',
        name: this.i18n.get('common_target_disk_position_name_label'),
        cellRender: this.diskDeviceTpl
      }
    ];
    this.restoreTableConfig = {
      table: {
        async: false,
        columns: colsLeft,
        compareWith: 'id',
        colDisplayControl: false,
        scroll: { y: '380px' },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: selection => {
          each(selection, item => {
            assign(item, {
              nameId: `${item.name || '--'}(${item.uuid})`,
              recoveryType: '1'
            });
          });
          const canceledDisk = differenceBy(
            this.targetTableData?.data,
            selection,
            'uuid'
          );
          this.cacheSelectedDisk = reject(this.cacheSelectedDisk, item =>
            includes(map(canceledDisk, 'targetDisk'), item)
          );
          this.targetTableData = {
            data: selection,
            total: size(selection)
          };
          if (!isEmpty(this.targetDisksOptions)) {
            each(this.targetTableData.data, item => {
              let targetOptions = cloneDeep(
                filter(
                  this.targetDisksOptions,
                  value =>
                    value.uuid === item.targetDisk ||
                    this.fiterDisk(value, item)
                )
              );
              each(targetOptions, val => {
                if (
                  val.size < item.size ||
                  find(this.cacheSelectedDisk, disk => disk === val.uuid)
                ) {
                  assign(val, {
                    disabled: true
                  });
                }
              });
              assign(item, {
                targetDiskOptions: targetOptions
              });
              if (
                this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
              ) {
                // 原位置可自动匹配符合条件的原磁盘
                each(targetOptions, val => {
                  if (
                    val.size >= item.size &&
                    item.uuid === val.uuid &&
                    !item.targetDisk
                  ) {
                    item.targetDisk = val.uuid;
                    this.diskChange();
                  }
                });
              }
            });
          }
          this.setVaild();
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        showTotal: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };

    this.targetTableConfig = {
      table: {
        columns: colsRight,
        async: false,
        colDisplayControl: false,
        scroll: { y: '380px' }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };

    defer(() => this.getLeftTableData());
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreTo: new FormControl(RestoreV2LocationType.ORIGIN),
      targetServer: new FormControl([]),
      proxyHost: new FormControl([]),
      restoreAutoPowerOn: new FormControl(false),
      copyVerify: new FormControl(false)
    });

    this.formGroup.get('restoreTo').valueChanges.subscribe(res => {
      this.clearTargetDisk();
      if (res === RestoreV2LocationType.ORIGIN) {
        this.formGroup.get('targetServer').clearValidators();
        defer(() => this.getResourceDetail());
      } else {
        this.getTreeData();
        this.formGroup
          .get('targetServer')
          .setValidators([this.baseUtilService.VALID.required()]);
      }
      this.formGroup.get('targetServer').updateValueAndValidity();
    });

    this.formGroup.get('targetServer').valueChanges.subscribe(res => {
      if (isEmpty(res)) {
        return;
      }
      this.getResourceDetail(res);
    });

    if (this.restoreToNewLocationOnly) {
      defer(() =>
        this.formGroup.get('restoreTo').setValue(RestoreV2LocationType.NEW)
      );
    }
  }

  clearTargetDisk() {
    each(this.targetTableData?.data, item => {
      item.targetDisk = '';
      assign(item, {
        targetDiskOptions: []
      });
    });
    this.setVaild();
  }

  getTargetDisk() {
    if (!isEmpty(this.targetDisksOptions)) {
      this.targetDisksOptions = map(this.targetDisksOptions, disk => {
        return assign(disk, {
          value: disk.uuid,
          key: disk.uuid,
          label: `${disk.name || '--'}(${disk.uuid})`,
          isLeaf: true
        });
      });
    }

    if (!isEmpty(this.targetTableData?.data)) {
      each(this.targetTableData?.data, item => {
        item.targetDisk = '';
        let targetOptions = cloneDeep(
          filter(this.targetDisksOptions, value => this.fiterDisk(value, item))
        );
        each(targetOptions, val => {
          if (val.size < item.size) {
            assign(val, {
              disabled: true
            });
          }
        });
        assign(item, {
          targetDiskOptions: targetOptions
        });
        if (this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN) {
          // 原位置可自动匹配符合条件的原磁盘
          each(targetOptions, val => {
            if (
              val.size >= item.size &&
              item.uuid === val.uuid &&
              !item.targetDisk
            ) {
              item.targetDisk = val.uuid;
              this.diskChange();
            }
          });
        }
      });
    }

    this.setVaild();
  }

  getResourceDetail(server?) {
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        queryDependency: true,
        conditions: JSON.stringify({
          uuid: !server
            ? this.resourceProperties.rootUuid ||
              this.resourceProperties.root_uuid
            : server[0].rootUuid
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
          if (!server) {
            this.getDisk(agentsId, res.records[0]);
          } else {
            this.getDisk(agentsId, res.records[0], server);
          }
        }
      });
  }

  getDisk(
    agentsId,
    resourceInfo,
    server?,
    recordsTemp?: any[],
    startPage?: number
  ) {
    const params = {
      agentId: agentsId,
      envId: !server
        ? this.resourceProperties.rootUuid || this.resourceProperties.root_uuid
        : server[0].rootUuid,
      resourceIds: [
        !server
          ? this.resourceProperties.uuid || this.resourceProperties.root_uuid
          : server[0].rootUuid
      ],
      pageNo: startPage || 1,
      pageSize: 200,
      conditions: JSON.stringify({
        resourceType: 'APS-disk',
        uuid: !server ? this.resourceProperties.uuid : server[0].uuid,
        regionId: !server
          ? this.resourceProperties.extendInfo.regionId
          : server[0].extendInfo.regionId
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
        this.targetDisksOptions = recordsTemp;
        this.getTargetDisk();
        return;
      }
      startPage++;
      if (!server) {
        this.getDisk(agentsId, resourceInfo, null, recordsTemp, startPage);
      } else {
        this.getDisk(agentsId, resourceInfo, server, recordsTemp, startPage);
      }
    });
  }

  getOptions(subType, params, node?) {
    const extParams = {
      conditions: JSON.stringify(params)
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        if (subType === DataMap.Resource_Type.ApsaraStack.value) {
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
          if (subType === DataMap.Resource_Type.APSZone.value) {
            resource = filter(resource, val => {
              return val.subType !== DataMap.Resource_Type.APSResourceSet.value;
            });
          }
          each(resource, item => {
            const isCloudServer =
              item.subType === DataMap.Resource_Type.APSCloudServer.value;
            node.children.push(
              assign(item, {
                label: item.name,
                disabled: !isCloudServer,
                children: isCloudServer ? null : [],
                isLeaf: isCloudServer,
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
    this.getOptions(DataMap.Resource_Type.ApsaraStack.value, {
      type: DataMap.Resource_Type.ApsaraStack.value,
      subType: DataMap.Resource_Type.ApsaraStack.value
    });
  }

  expandedChange(node) {
    if (!node.expanded || node.children?.length) {
      return;
    }
    node.children = [];
    if (node.subType === DataMap.Resource_Type.ApsaraStack.value) {
      this.getOptions(
        DataMap.Resource_Type.APSRegion.value,
        {
          parentUuid: node.uuid
        },
        node
      );
    } else if (node.subType === DataMap.Resource_Type.APSRegion.value) {
      this.getOptions(
        DataMap.Resource_Type.APSZone.value,
        {
          parentUuid: node.uuid
        },
        node
      );
    } else {
      this.getOptions(
        DataMap.Resource_Type.APSCloudServer.value,
        {
          path: [['=~'], `${node.path}/`],
          subType: [DataMap.Resource_Type.APSCloudServer.value]
        },
        node
      );
    }
  }

  getLeftTableData() {
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
        ),
        mode: item.bootable === 'system' ? 'true' : 'false'
      });
    });
    this.restoreTableData = {
      data: needRestoreDisks,
      total: size(needRestoreDisks)
    };
  }

  fiterDisk(value, item) {
    // 数据盘不能恢复到系统盘, portable为false不能用
    if (!value.extendInfo.portable) {
      return false;
    }
    if (item.mode === DataMap.Disk_Mode.false.value) {
      return value.mode === DataMap.Disk_Mode.false.value;
    }
    return true;
  }

  setVaild() {
    this.disk$.next(
      !isEmpty(this.targetTableData?.data) &&
        this.formGroup.valid &&
        every(this.targetTableData?.data, item => {
          return item.recoveryType === this.EXIST_DISK
            ? !isEmpty(item.targetDisk)
            : true;
        })
    );
  }

  diskChange(_?, disk?) {
    this.cacheSelectedDisk = reject(
      map(this.targetTableData?.data, 'targetDisk'),
      item => isEmpty(item)
    );
    each(this.targetTableData?.data, item => {
      let tmpOptions = item.targetDiskOptions;
      each(tmpOptions, val => {
        if (
          find(this.cacheSelectedDisk, disk => disk === val.uuid) ||
          item.size > val.size
        ) {
          val.disabled = true;
        } else {
          val.disabled = false;
        }
      });
      item.targetDiskOptions = [...tmpOptions];
    });
    this.setVaild();
  }

  getTargetPath() {
    return this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
      ? this.resourceProperties.path
      : this.formGroup.value.targetServer[0]?.path;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        copyId: this.rowCopy.uuid,
        agents: this.formGroup.value.proxyHost,
        targetEnv:
          this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
            ? this.resourceProperties?.environment_uuid
            : this.formGroup.value.targetServer[0]?.rootUuid,
        restoreType: this.restoreType,
        targetLocation: this.formGroup.value.restoreTo,
        subObjects: map(this.targetTableData.data, item => {
          let curData;
          if (item.recoveryType === this.EXIST_DISK) {
            curData = pick(
              find(this.targetDisksOptions, {
                uuid: item.targetDisk
              }),
              ['uuid', 'size']
            );
            curData.id = curData.uuid;
            curData.isNewDisk = false;
            delete curData.uuid;
          } else {
            curData = {
              id: '',
              size: toNumber(item.size),
              isNewDisk: 'true'
            };
          }

          return JSON.stringify({
            uuid: item.uuid,
            extendInfo: {
              targetVolume: JSON.stringify(curData)
            }
          });
        }),
        targetObject:
          this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
            ? this.resourceProperties.uuid
            : this.formGroup.value.targetServer[0]?.uuid,
        extendInfo: {
          restoreLevel: 1,
          powerState: this.formGroup.value.restoreAutoPowerOn ? '1' : '0',
          copyVerify: this.formGroup.value.copyVerify ? 'true' : 'false',
          restoreLocation:
            this.formGroup.value.restoreTo === RestoreV2LocationType.ORIGIN
              ? this.resourceProperties.path
              : this.formGroup.value.targetServer[0]?.path
        }
      };
      if (this.rowCopy.status === DataMap.copydata_validStatus.invalid.value) {
        assign(params.extendInfo, {
          force_recovery: true
        });
      }
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params })
        .subscribe(
          () => {
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
