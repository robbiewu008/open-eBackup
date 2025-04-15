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
import { getBootTypeWarnTipByType, MODAL_COMMON } from 'app/shared';
import {
  AppService,
  ProtectedResourceApiService,
  RestoreApiV2Service
} from 'app/shared/api/services';
import {
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  LANGUAGE,
  ResourceType,
  RestoreFileType,
  RestoreLocationType,
  RestoreV2LocationType,
  SYSTEM_TIME
} from 'app/shared/consts';
import { I18NService } from 'app/shared/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  find,
  first,
  get,
  isArray,
  isEmpty,
  isString,
  isUndefined,
  map,
  omit,
  size
} from 'lodash';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { TargetLocationComponent } from './target-location/target-location.component';
import { FormGroup } from '@angular/forms';
import { forkJoin, Observable, Observer, of, Subject } from 'rxjs';
import { MessageService } from '@iux/live';
import { mergeMap } from 'rxjs/operators';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
@Component({
  selector: 'aui-fusion-compute-disk-restore',
  templateUrl: './fusion-compute-disk-restore.component.html',
  styleUrls: ['./fusion-compute-disk-restore.component.less']
})
export class FusionComputeDiskRestoreComponent
  implements OnInit, AfterViewInit {
  @Input() rowCopy;
  @Input() restoreType;
  restoreFileType = RestoreFileType;
  restoreV2LocationType = RestoreV2LocationType;
  restoreLocationType = RestoreLocationType;
  language = LANGUAGE;
  unitConst = CAPACITY_UNIT;
  targetParams;
  inputTarget = '';
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('selectTable', { static: false }) selectTable: ProTableComponent;
  @ViewChild('deskDeviceTpl', { static: true }) deskDeviceTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('dataStoreSizeTpl', { static: true })
  dataStoreSizeTpl: TemplateRef<any>;
  @ViewChild('helpExtraTpl', { static: true }) helpExtraTpl: TemplateRef<any>;
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
  copySelectData = {
    data: [],
    total: 0
  };
  resourceProp;
  properties;
  targetDiskOptions = []; // 目标磁盘位置
  disk$ = new Subject<boolean>();
  selectedLocation = this.restoreV2LocationType.ORIGIN;
  _isUndefined = isUndefined;
  _isEmpty = isEmpty;
  rowCopyBootType; // 副本中记录的bootType，有可能为空
  bootOptionsWarnTip;
  targetDataStoreErrorTip = this.i18n.get(
    'protection_restore_new_datastore_tip_label'
  );
  isOriginPosition = true;
  isSame = false;
  sameDataStore;
  calcSize = 0;
  originalResource;

  constructor(
    public i18n: I18NService,
    private restoreV2Service: RestoreApiV2Service,
    private drawModalService: DrawModalService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private messageService: MessageService,
    private appService: AppService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.initData();
    this.getOriginalResource();
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
        key: 'slot',
        name: this.i18n.get('common_slot_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'size',
        name: this.i18n.get('protection_disk_data_capacity_label'),
        cellRender: this.sizeTpl
      },
      {
        key: 'datastore',
        name: this.i18n.get('common_datastore_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];

    const colsRight: TableCols[] = [
      {
        key: 'name',
        width: 140,
        name: this.i18n.get('common_restore_disk_name_label')
      },
      {
        key: 'datastore',
        name: this.i18n.get('protection_tagert_database_label'),
        width: 200,
        cellRender: this.deskDeviceTpl
      },
      {
        key: 'size',
        name: this.i18n.get('protection_tagert_database_capacity_label'),
        thExtra: this.helpExtraTpl,
        cellRender: this.dataStoreSizeTpl
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
            assign(item, { requiredSize: +item.size });
          });
          if (this.isOriginPosition && !isEmpty(this.inputTarget)) {
            if (!isEmpty(selection)) {
              this.disk$.next(true);
            } else {
              this.disk$.next(false);
            }
          }
          this.copySelectData = cloneDeep(this.selectData); // copySelectData存储上一次选择的数据

          if (!this.isOriginPosition) {
            if (!this.isSame) {
              selection = selection.map(item => omit(item, 'size'));
              each(selection, item => {
                const findItem = find(this.copySelectData.data, {
                  id: item.id
                });
                if (!isEmpty(findItem)) {
                  assign(item, {
                    size: findItem.size,
                    targetDisk: findItem.targetDisk,
                    targetDiskOptions: findItem.targetDiskOptions,
                    targetDiskName: findItem.targetDiskName
                  });
                }
              });
              this.copySelectData.data = cloneDeep(selection);
              this.selectData = {
                data: selection,
                total: size(selection)
              };
              this.getShowTableDiskSize();
            } else {
              this.selectData = {
                data: cloneDeep(selection),
                total: size(selection)
              };
              this.calcSameStoreSize();
            }
          } else {
            selection = selection.map(item => omit(item, 'size'));
            this.selectData = {
              data: cloneDeep(selection),
              total: size(selection)
            };
            this.calcOriginShowSize();
          }

          if (!this.isOriginPosition && !isEmpty(this.inputTarget)) {
            if (this.isSame) {
              this.setSameValid();
            } else {
              this.setValid();
            }
          }
        },
        trackByFn: (index, item) => {
          return item.id;
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
        scroll: { y: '680px' },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };
  }

  getShowTableDiskSize() {
    const dataStoreInfo = {};
    each(this.selectData.data, item => {
      if (!isEmpty(item.targetDisk)) {
        const findItem = find(cloneDeep(this.targetDiskOptions), {
          id: item.targetDisk
        });
        if (isUndefined(dataStoreInfo[findItem.id])) {
          dataStoreInfo[findItem.id] = item.requiredSize;
        } else {
          dataStoreInfo[findItem.id] += item.requiredSize;
        }
      }
    });

    each(this.selectData.data, curData => {
      if (!isEmpty(curData.targetDisk)) {
        const findItem = find(cloneDeep(this.targetDiskOptions), {
          id: curData.targetDisk
        });
        curData.size = findItem.size - dataStoreInfo[curData.targetDisk];
        curData.errorTip = curData.size < 0 ? this.targetDataStoreErrorTip : '';
      }
    });
  }

  initData() {
    this.resourceProp = JSON.parse(this.rowCopy.resource_properties);
    this.properties = JSON.parse(this.rowCopy.properties);
    this.rowCopyBootType = get(this.properties, 'bootType', null);
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

    const diskData = isString(this.properties?.disk_info)
      ? JSON.parse(this.properties?.disk_info || '{}')
      : this.properties?.disk_info;
    if (diskData?.length) {
      const showData = diskData.map(item => {
        return {
          id: item.uuid,
          name: item.name,
          slot: `${item.pciType}(${item.sequenceNum})`,
          size: item.quantityGB,
          datastore: item.datastoreName,
          datastoreUrn: item.datastoreUrn
        };
      });
      this.tableData = {
        data: showData,
        total: size(showData)
      };
    }
  }

  getBootOptionsTip(rowData?) {
    getBootTypeWarnTipByType(this, rowData?.bootType, this.rowCopyBootType);
  }

  getOriginalResource() {
    this.protectedResourceApiService
      .ShowResource({
        resourceId: this.resourceProp?.uuid,
        akDoException: false
      })
      .subscribe({
        next: res => {
          this.originalResource = res;
        },
        error: () => {
          this.originalResource = this.resourceProp;
        }
      });
  }

  diskChange(diskId, data) {
    this.getShowTableDiskSize();
    assign(data, {
      preDisk: data.targetDisk,
      targetDiskName: find(this.targetDiskOptions, { id: diskId })?.name
    });

    this.setValid();
  }

  setValid() {
    let flag = false;

    if (!this.selectData.data.length) {
      this.disk$.next(false);
      return;
    }
    for (const item of this.selectData.data) {
      if (isEmpty(item.targetDisk)) {
        flag = true;
        break;
      }
      if (item?.size < 0) {
        this.messageService.error(
          this.i18n.get('common_select_fc_disk_label'),
          {
            lvMessageKey: 'fc_disk_restore',
            lvShowCloseButton: true
          }
        );
        flag = true;
        break;
      }
    }

    if (flag) {
      this.disk$.next(false);
    } else {
      this.disk$.next(true);
    }
  }

  selectRecoveryTarget() {
    this.disk$.next(false); // 点击了请选择直接页面置灰
    this.protectedResourceApiService
      .ListResources({
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          subType:
            this.rowCopy.resource_sub_type ===
            DataMap.Resource_Type.fusionOne.value
              ? ResourceType.FUSION_ONE
              : ResourceType.FUSION_COMPUTE,
          uuid: this.resourceProp.root_uuid
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
            existEnv: !!res.totalCount
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
            this.inputTarget = this.targetParams.path;
            if (this.selectedLocation === this.restoreV2LocationType.ORIGIN) {
              this.bootOptionsWarnTip = '';
              this.isOriginPosition = true;
              this.getDataStores().subscribe(response => {
                const totalData = [...response];
                if (totalData.length) {
                  this.targetDiskOptions = map(totalData, item => {
                    return assign(item, {
                      id: item.uuid,
                      isLeaf: true,
                      label: item.name,
                      size: item?.extendInfo?.freeSizeGB,
                      datastoreUrn: item?.extendInfo?.datastoreUrn
                    });
                  });
                  this.calcOriginShowSize();
                }
              });
            } else {
              this.isOriginPosition = false;
              this.getBootOptionsTip(this.targetParams);
              if (this.targetParams.isSame) {
                this.sameDataStore = this.targetParams.dataStore[0];
                this.isSame = true;
                this.calcSameStoreSize();
              } else {
                const showData = this.selectData.data.map(item =>
                  omit(item, ['size', 'targetDisk', 'targetDiskOptions'])
                );
                this.selectData = {
                  data: cloneDeep(showData),
                  total: size(showData)
                };
                this.targetDiskOptions = this.targetParams.dataStore;
                this.isSame = false;
              }
              delete this.targetParams.bootType;
            }
          }
        });
      });
  }

  calcSameStoreSize() {
    const copyData = cloneDeep(this.selectData.data);
    const usedSize = copyData.reduce((pre, cur) => {
      return pre + cur.requiredSize;
    }, 0);
    this.calcSize = this.sameDataStore.size - usedSize;
    this.setSameValid();
  }

  calcOriginShowSize() {
    const usedSize = this.selectData.data.reduce((pre, cur) => {
      return pre + cur.requiredSize;
    }, 0);
    each(this.selectData.data, curData => {
      const findItem = find(this.targetDiskOptions, {
        datastoreUrn: curData.datastoreUrn
      });
      if (!isEmpty(findItem)) {
        curData.size = Number(findItem.size) - usedSize;
        curData.errorTip = curData.size < 0 ? this.targetDataStoreErrorTip : '';
      }
    });
    this.setOriginValid();
  }

  setOriginValid() {
    let flag = false;

    if (!this.selectData.data.length) {
      this.disk$.next(false);
      return;
    }
    for (const item of this.selectData.data) {
      if (isUndefined(item.size)) {
        flag = true;
        break;
      }
      if (item?.size < 0) {
        this.messageService.error(
          this.i18n.get('common_select_fc_disk_label'),
          {
            lvMessageKey: 'fc_disk_restore',
            lvShowCloseButton: true
          }
        );
        flag = true;
        break;
      }
    }

    if (flag) {
      this.disk$.next(false);
    } else {
      this.disk$.next(true);
    }
  }

  setSameValid() {
    if (!this.selectData.data.length) {
      this.disk$.next(false);
      return;
    }

    if (this.calcSize < 0) {
      this.messageService.error(this.i18n.get('common_select_fc_disk_label'), {
        lvMessageKey: 'fc_disk_restore',
        lvShowCloseButton: true
      });
      this.disk$.next(false);
    } else {
      this.disk$.next(true);
    }
  }

  getDataStores(): Observable<any> {
    const resource = {
      rootUuid: this.resourceProp.environment_uuid,
      uuid: this.resourceProp.parent_uuid
    };
    return new Observable<any>((observer: Observer<any>) => {
      this.appUtilsService
        .getResourcesDetails(resource, '', {}, {}, false)
        .subscribe(res => {
          observer.next(res);
          observer.complete();
        });
    });
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      let params = {};
      const selectDiskObject = this.selectData.data.map(item => item.id);
      if (this.selectedLocation === RestoreV2LocationType.ORIGIN) {
        params = {
          copyId: this.rowCopy.uuid,
          agents: this.targetParams.agents || [],
          targetEnv: this.resourceProp?.environment_uuid,
          restoreType: this.restoreType,
          targetLocation: this.selectedLocation,
          subObjects: selectDiskObject,
          targetObject: this.resourceProp?.uuid,
          extendInfo: {
            is_power_on:
              this.targetParams.extendInfo?.power_on === 'true'
                ? 'true'
                : 'false',
            is_in_place: 'true',
            is_disk_restore: 'true',
            restoreLocation: this.inputTarget,
            location: this.originalResource?.extendInfo?.location,
            copyVerify: this.targetParams.extendInfo?.copyVerify
          }
        };
      } else {
        params = {
          copyId: this.rowCopy.uuid,
          agents: this.targetParams.agents || [],
          targetEnv: this.targetParams.environment.uuid,
          restoreType: this.restoreType,
          targetLocation: this.selectedLocation,
          subObjects: selectDiskObject,
          targetObject: this.targetParams.vm.uuid,
          extendInfo: {
            is_power_on:
              this.targetParams.extendInfo?.power_on === 'true'
                ? 'true'
                : 'false',
            is_in_place: 'false',
            is_disk_restore: 'true',
            restoreLocation: this.inputTarget,
            location: this.targetParams.location,
            copyVerify: this.targetParams.extendInfo?.copyVerify
          }
        };

        if (this.isSame) {
          each(this.selectData.data, (item: any) => {
            params['extendInfo'][item.id] = this.sameDataStore.moReference;
          });
        } else {
          each(this.selectData.data, item => {
            const usedDisk = find(this.targetDiskOptions, {
              id: item.targetDisk
            });
            params['extendInfo'][item.id] = usedDisk.moReference;
          });
        }
      }
      if (this.rowCopy.status === DataMap.copydata_validStatus.invalid.value) {
        params['extendInfo']['force_recovery'] = true;
      }
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

  getTargetPath() {
    return this.inputTarget;
  }
}
