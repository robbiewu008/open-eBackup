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
  AfterViewInit,
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MODAL_COMMON } from 'app/shared';
import {
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  CommonConsts,
  DataMap,
  LANGUAGE,
  RestoreFileType,
  RestoreLocationType,
  RestoreV2LocationType
} from 'app/shared/consts';
import {
  CookieService,
  DataMapService,
  I18NService
} from 'app/shared/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  defer,
  each,
  eq,
  find,
  isEmpty,
  isNil,
  isString,
  map,
  remove,
  set,
  size,
  some
} from 'lodash';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { TargetLocationComponent } from './target-location/target-location.component';
import { FormGroup } from '@angular/forms';
import { Observable, Observer, Subject } from 'rxjs';
import { MessageService } from '@iux/live';

@Component({
  selector: 'aui-hcs-restore',
  templateUrl: './hcs-restore.component.html',
  styleUrls: ['./hcs-restore.component.less']
})
export class HCSRestoreComponent
  implements OnInit, AfterViewInit, AfterViewInit {
  @Input() rowCopy;
  @Input() restoreType;
  restoreFileType = RestoreFileType;
  restoreV2LocationType = RestoreV2LocationType;
  restoreLocationType = RestoreLocationType;
  language = LANGUAGE;
  targetParams;
  inputTarget = '';
  copyInputTarget = '';
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('selectTable', { static: false }) selectTable: ProTableComponent;
  @ViewChild('deskDeviceTpl', { static: true }) deskDeviceTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('diskHelpTpl', { static: true }) diskHelpTpl: TemplateRef<any>;
  @ViewChild('encryptedTpl', { static: true }) encryptedTpl: TemplateRef<any>;

  formGroup: FormGroup;
  tableConfig: TableConfig;
  datasTableConfig: TableConfig;
  tableData = {
    data: [],
    total: 0
  };
  selectData = {
    data: [],
    total: 0
  };
  resourceProp;
  properties;
  targetDiskOptions = []; // 目标磁盘位置
  copyTargetDiskOptions = [];
  disk$ = new Subject<boolean>();
  selectedLocation = this.restoreV2LocationType.ORIGIN;
  usedTotalDisk = new Set();
  _isEmpty = isEmpty;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  constructor(
    public i18n: I18NService,
    private cookieService: CookieService,
    private restoreV2Service: RestoreApiV2Service,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private messageService: MessageService
  ) {}

  ngOnInit() {
    this.initDeployType();
    this.initConfig();
    this.initData();
  }

  initDeployType() {
    const resourceProperties = JSON.parse(this.rowCopy.resource_properties);
    this.isHcsUser =
      resourceProperties.environment_sub_type ===
      DataMap.Job_Target_Type.hcsEnvOp.value;
  }

  ngAfterViewInit() {
    if (this.dataTable) {
      this.dataTable.fetchData();
    }
    if (this.selectTable) {
      this.selectTable.fetchData();
    }
  }

  initConfig() {
    const colsLeft: TableCols[] = [
      {
        key: 'name',
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
        key: 'attr',
        name: this.i18n.get('protection_incremental_mode_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Disk_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Disk_Status')
        }
      },
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.sizeTpl
      },
      {
        key: 'systemEncrypted',
        name: this.i18n.get('protection_hcs_encryption_label'),
        cellRender: this.encryptedTpl
      }
    ];

    const colsRight: TableCols[] = [
      {
        key: 'name',
        width: 170,
        name: this.i18n.get('common_restore_object_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'position',
        name: this.i18n.get('common_target_cloud_disk_name_label'),
        cellRender: this.deskDeviceTpl,
        thExtra: this.diskHelpTpl
      }
    ];
    this.datasTableConfig = {
      table: {
        async: false,
        columns: colsLeft,
        compareWith: 'id',
        colDisplayControl: false,
        scroll: { y: '680px' },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: selection => {
          each(selection, item => {
            const findItem = find(this.selectData.data, { id: item.id });
            if (isEmpty(findItem)) {
              this.selectData.data.push(item);
              defer(() => this.initDeskDeviceTargetDisk());
            }
          });

          remove(this.selectData.data, item => {
            return isEmpty(find(selection, { id: item.id }));
          });

          this.usedTotalDisk.clear();
          each(this.selectData.data, item => {
            if (!isEmpty(item.targetDisk)) {
              each(item.targetDisk, itemDiskId => {
                this.usedTotalDisk.add(itemDiskId);
              });
            }
          });
          each(this.selectData.data, item => {
            const {
              originDiskOptions,
              targetDiskOptions
            } = this.getSelectOptionData(item);
            assign(item, {
              requiredSize: +item.size,
              targetDiskOptions,
              originDiskOptions
            });
          });

          this.selectData = {
            data: cloneDeep(this.selectData.data),
            total: size(this.selectData.data)
          };
          this.setValid();
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        showTotal: true
      }
    };

    this.tableConfig = {
      table: {
        columns: colsRight,
        async: false,
        size: 'small',
        colDisplayControl: false,
        scroll: { y: '680px' }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
  }

  getFilterData(diskOptions, item) {
    const lDisk = find(this.tableData.data, { id: item.id });
    if (lDisk.architecture === '') {
      diskOptions = diskOptions.filter(cur =>
        ['', 'x86_64'].includes(cur.architecture)
      );
    } else {
      diskOptions = diskOptions.filter(
        cur =>
          cur.architecture === lDisk.architecture || cur.architecture === ''
      );
    }
    if (lDisk.mode === 'false') {
      diskOptions = diskOptions.filter(cur => cur.mode === 'false');
    }

    diskOptions = diskOptions.filter(
      cur => Number(cur.size) >= Number(lDisk.size)
    );

    return diskOptions;
  }

  getSelectOptionData(item) {
    let targetDiskOptions = this.copyTargetDiskOptions.filter(
      cur =>
        item.targetDisk?.includes(cur.id) || !this.usedTotalDisk.has(cur.id)
    );
    let originDiskOptions = cloneDeep(this.copyTargetDiskOptions);

    originDiskOptions = this.getFilterData(originDiskOptions, item);
    targetDiskOptions = this.getFilterData(targetDiskOptions, item);
    return {
      originDiskOptions,
      targetDiskOptions
    };
  }

  initData() {
    this.resourceProp = JSON.parse(this.rowCopy.resource_properties);
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
      isEmpty(this.properties.volList)
    ) {
      assign(this.properties, {
        volList: this.properties.extendInfo?.volList
      });
    }
    if (this.properties?.volList?.length) {
      const showData = this.properties?.volList.map(item => {
        const data = isString(item.extendInfo)
          ? JSON.parse(item.extendInfo || '{}')
          : item.extendInfo;
        return {
          id: item.uuid,
          name: item.name,
          ...data
        };
      });
      this.tableData = {
        data: showData,
        total: size(showData)
      };
    }
  }

  diskChange(diskId, data) {
    this.usedTotalDisk.clear();
    each(this.selectData.data, item => {
      each(item.targetDisk, itemDiskId => {
        this.usedTotalDisk.add(itemDiskId);
      });
    });

    each(this.selectData.data, item => {
      if (item.id !== data.id) {
        item.targetDiskOptions = item.originDiskOptions.filter(
          cur =>
            item.targetDisk?.includes(cur.id) || !this.usedTotalDisk.has(cur.id)
        );
      }
    });

    this.setValid();
  }
  /*
   * 1、加密盘只能选择恢复到自己
   * 2、非加密盘可以恢复到自己 或 其他的非加密盘
   * 3、升级上来的副本 不存在加密字段，所以可以随便选择
   * '1':代表加密
   * ''或'0'代表未加密
   * */
  setValid() {
    let inValidFlag = false;
    if (!this.selectData.data.length) {
      this.disk$.next(false);
      return;
    }
    for (const item of this.selectData.data) {
      if (isEmpty(item.targetDisk)) {
        inValidFlag = true;
        break;
      }
      // 老副本不存在该字段 不用判断
      if (isNil(item.systemEncrypted)) {
        continue;
      }

      // '1'加密 其余都是非加密
      const encrypted = item.systemEncrypted === '1';

      if (encrypted) {
        // 加密盘只能选择恢复到自己 所以多选的都无需判断
        // 加密盘这里只允许一个目标，some是考虑后续拓展
        inValidFlag =
          item.targetDisk.length > 1 ||
          some(item.targetDisk, itemDiskId => {
            const disk = find(item.originDiskOptions, { id: itemDiskId });
            return (
              disk.id !== item.id ||
              disk.systemEncrypted !== '1' ||
              disk.cipher !== item.cipher ||
              disk.systemCmkId !== item.systemCmkId
            );
          });
      } else {
        // 升级的副本也走这
        // 非加密盘除了加密盘之外，都能选
        inValidFlag = some(item.targetDisk, itemDiskId => {
          const disk = find(item.originDiskOptions, { id: itemDiskId });
          return disk.systemEncrypted === '1';
        });
      }

      if (inValidFlag) {
        const errorStr = encrypted
          ? 'common_select_encrypted_hcs_disk_label'
          : 'common_select_not_encrypted_hcs_disk_label';
        this.messageService.error(this.i18n.get(errorStr), {
          lvMessageKey: 'hcs_disk_restore',
          lvShowCloseButton: true
        });
        break;
      }
      if (item?.size < 0) {
        this.messageService.error(
          this.i18n.get('common_select_hcs_disk_label'),
          {
            lvMessageKey: 'hcs_disk_restore',
            lvShowCloseButton: true
          }
        );

        inValidFlag = true;
        break;
      }
    }

    this.disk$.next(!inValidFlag);
  }

  selectRecoveryTarget() {
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          uuid: this.resourceProp?.parent_uuid
        })
      })
      .subscribe(res => {
        this.drawModalService.create({
          ...MODAL_COMMON.generateDrawerOptions(),
          lvWidth: MODAL_COMMON.normalWidth + 100,
          lvHeader: this.i18n.get('protection_select_restore_target_label'),
          lvOkDisabled: false,
          lvContent: TargetLocationComponent,
          lvComponentParams: {
            rowCopy: this.rowCopy,
            params: this.targetParams,
            position: this.selectedLocation,
            existEnv: !!res.totalCount,
            verifyStatus: this.properties?.verifyStatus
          },
          lvAfterOpen: modal => {
            const content = modal.getContentComponent();
            const modalIns = modal.getInstance();
            content.formGroup.statusChanges.subscribe(res => {
              modalIns.lvOkDisabled = res !== 'VALID';
            });
          },
          lvOk: modal => {
            const content = modal.getContentComponent();
            this.targetParams = content.getTargetParams();
            this.selectedLocation = content.formGroup.value.restoreLocation;
            const path = this.resourceProp.path.replace(
              this.resourceProp.name,
              ''
            );
            this.inputTarget = path + this.targetParams.cloudHost?.label;
            // 新位置
            if (this.selectedLocation === RestoreV2LocationType.NEW) {
              this.inputTarget = this.targetParams.cloudHost?.path;
            }

            defer(() => this.initDeskDeviceTargetDisk(true));

            if (
              isEmpty(this.copyInputTarget) ||
              this.copyInputTarget !== this.inputTarget
            ) {
              this.copyInputTarget = this.inputTarget;
              const data = JSON.parse(
                this.targetParams?.cloudHost?.extendInfo?.host || '{}'
              );

              this.targetDiskOptions = map(data.diskInfo, item => {
                return assign(item, { isLeaf: true, label: item.name });
              });

              this.copyTargetDiskOptions = this.targetDiskOptions;
              this.usedTotalDisk.clear();
              each(this.selectData.data, item => {
                const {
                  originDiskOptions,
                  targetDiskOptions
                } = this.getSelectOptionData(item);
                assign(item, {
                  requiredSize: +item.size,
                  targetDiskOptions,
                  originDiskOptions,
                  targetDisk: []
                });
              });

              this.selectData = {
                data: cloneDeep(this.selectData.data),
                total: size(this.selectData.data)
              };
            } else {
              this.setValid();
            }
          }
        });
      });
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        copyId: this.rowCopy.uuid,
        agents: this.targetParams.agents,
        targetEnv:
          this.selectedLocation === RestoreV2LocationType.ORIGIN
            ? this.resourceProp?.environment_uuid
            : this.targetParams.cloudHost.rootUuid,
        restoreType: this.restoreType,
        targetLocation: this.selectedLocation,
        subObjects: map(this.selectData.data, item => {
          const totalDiskInfo = [];
          each(item.targetDisk, diskId => {
            const curData = find(this.targetDiskOptions, { id: diskId });
            totalDiskInfo.push({
              uuid: curData.id,
              lunWWN: curData.lunWWN,
              size: curData.size
            });
          });
          return JSON.stringify({
            uuid: item.id,
            extendInfo: {
              targetVolumes: JSON.stringify(totalDiskInfo)
            }
          });
        }),
        targetObject:
          this.selectedLocation === RestoreV2LocationType.ORIGIN
            ? this.resourceProp?.uuid
            : this.targetParams.cloudHost.uuid,
        extendInfo: {
          powerState: this.targetParams.restoreAutoPowerOn,
          copyVerify: this.targetParams.copyVerify,
          restoreLocation: this.inputTarget
        }
      };
      if (this.rowCopy.status === DataMap.copydata_validStatus.invalid.value) {
        params['extendInfo']['force_recovery'] = true;
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
  }

  getTargetPath() {
    return this.inputTarget;
  }

  /**
   * 初始化右侧目标云盘名称初始值（原位置时优化体验)
   * @param isReset 是否重置
   */
  initDeskDeviceTargetDisk(isReset = false) {
    if (
      eq(this.selectedLocation, this.restoreV2LocationType.NEW) ||
      isEmpty(this.inputTarget)
    ) {
      this.setValid();
      return;
    }
    const data = map(cloneDeep(this.selectData.data), item => {
      if (isReset || isNil(item.targetDisk)) {
        const params = {
          ...item,
          targetDisk: find(item.targetDiskOptions, { id: item.id })
            ? [item.id]
            : [] // 磁盘选择互斥，所以塞值的时候要注意目标options是否存在该id
        };
        return params;
      }
      return item;
    });
    set(this, 'selectData', {
      data,
      total: size(data)
    });
    // 手动触发一次磁盘变更，便于过滤掉已选盘
    each(this.selectData?.data, item => this.diskChange(null, item));
    this.setValid();
  }
}
