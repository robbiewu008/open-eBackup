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
import {
  CAPACITY_UNIT,
  DataMap,
  DataMapService,
  getBootTypeWarnTipByType,
  I18NService,
  LANGUAGE,
  MODAL_COMMON,
  RestoreApiV2Service,
  RestoreV2LocationType
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  defer,
  differenceBy,
  each,
  every,
  filter,
  find,
  get,
  includes,
  isEmpty,
  map,
  omit,
  reject,
  set,
  size
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { HypervRestoreComponent } from '../hyperv-restore/hyperv-restore.component';

@Component({
  selector: 'aui-disk-restore',
  templateUrl: './disk-restore.component.html',
  styleUrls: ['./disk-restore.component.less']
})
export class DiskRestoreComponent implements OnInit {
  rowCopy;
  restoreType;

  language = LANGUAGE;
  unitconst = CAPACITY_UNIT;
  restoreLocationType = RestoreV2LocationType;
  _isEmpty = isEmpty;
  resourceProperties;
  properties;
  restoreTableConfig: TableConfig;
  restoreTableData: TableData;
  targetTableConfig: TableConfig;
  targetTableData: TableData;
  isOriginPosition = true;
  bootOptionsWarnTip;
  rowCopyBootType;
  targerLocationInput;
  targetParams;
  targetDisksOptions;
  cacheSelectedDisk = [];

  disk$ = new Subject<boolean>();

  @ViewChild('restoreTable', { static: false }) restoreTable: ProTableComponent;
  @ViewChild('targeTable', { static: false }) targeTable: ProTableComponent;
  @ViewChild('deskDeviceTpl', { static: true }) deskDeviceTpl: TemplateRef<any>;
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private restoreV2Service: RestoreApiV2Service
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  compareWithKey() {
    return 'uuid';
  }

  getSizeTpl() {
    return this.sizeTpl;
  }

  initConfig() {
    this.resourceProperties = JSON.parse(
      this.rowCopy?.resource_properties || '{}'
    );
    this.properties = JSON.parse(this.rowCopy?.properties || '{}');
    this.rowCopyBootType = get(this.properties, 'bootType', null);
    const colsLeft: TableCols[] = [
      {
        key: 'diskName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'type',
        name: this.i18n.get('common_type_label'),
        hidden: !includes(
          [DataMap.Resource_Type.hyperVVm.value],
          this.rowCopy.resource_sub_type
        )
      },
      {
        key: 'bootable',
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
        },
        hidden: true
      },
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.getSizeTpl()
      }
    ];

    const colsRight: TableCols[] = [
      {
        key: 'diskName',
        width: 170,
        name: this.i18n.get('common_restore_disk_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'position',
        name: this.i18n.get('common_target_disk_position_name_label'),
        hidden: includes(
          [DataMap.Resource_Type.hyperVVm.value],
          this.resourceProperties.sub_type
        ),
        cellRender: this.deskDeviceTpl
      },
      {
        key: 'position',
        name: this.i18n.get('protection_tagert_database_label'),
        hidden: !includes(
          [DataMap.Resource_Type.hyperVVm.value],
          this.resourceProperties.sub_type
        ),
        cellRender: this.deskDeviceTpl
      }
    ];
    this.restoreTableConfig = {
      table: {
        async: false,
        columns: colsLeft,
        compareWith: this.compareWithKey(),
        colDisplayControl: false,
        scroll: { y: '680px' },
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        selectionChange: selection => {
          const canceledDisk = differenceBy(
            this.targetTableData?.data,
            selection,
            this.compareWithKey()
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
              assign(item, {
                targetDiskOptions: filter(
                  this.targetDisksOptions,
                  value =>
                    value[this.compareWithKey()] === item.targetDisk ||
                    this.fiterDisk(value, item)
                )
              });
            });
          }
          this.setVaild();
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        showTotal: true
      }
    };

    this.targetTableConfig = {
      table: {
        columns: colsRight,
        async: false,
        colDisplayControl: false,
        scroll: { y: '680px' }
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };

    defer(() => this.getLeftTableData());
  }

  getRestoreTargetComponent() {
    return HypervRestoreComponent;
  }

  selectRecoveryTarget() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'select-restore-target-draw',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: this.i18n.get('protection_select_restore_target_label'),
        lvContent: this.getRestoreTargetComponent(),
        lvOkDisabled: true,
        lvComponentParams: {
          rowCopy: this.rowCopy,
          restoreType: this.restoreType,
          targetParams: this.targetParams
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent();
          content.formGroup.statusChanges.subscribe(res => {
            modal.lvOkDisabled = res !== 'VALID';
          });
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          const content = modal.getContentComponent();
          this.targetParams = content.getTargetParams();
          this.isOriginPosition =
            this.targetParams.formGroupValue.restoreTo ===
            RestoreV2LocationType.ORIGIN;
          this.targerLocationInput = this.targetParams?.requestParams?.extendInfo?.restoreLocation;
          this.targetDisksOptions = this.targetParams?.targetDisk;
          this.cacheSelectedDisk = [];
          if (this.isOriginPosition) {
            this.bootOptionsWarnTip = '';
          } else {
            getBootTypeWarnTipByType(
              this,
              this.targetParams?.bootType,
              this.rowCopyBootType
            );
          }
          if (!isEmpty(this.targetDisksOptions)) {
            this.targetDisksOptions = map(this.targetDisksOptions, disk => {
              return assign(disk, {
                value: disk[this.compareWithKey()],
                key: disk[this.compareWithKey()],
                label: disk.name,
                isLeaf: true
              });
            });
          }
          if (!isEmpty(this.targetTableData?.data)) {
            each(this.targetTableData?.data, item => {
              item.targetDisk = '';
              assign(item, {
                targetDiskOptions: filter(
                  cloneDeep(this.targetDisksOptions),
                  val => this.fiterDisk(val, item)
                )
              });
            });
          }
          this.setVaild();
        }
      })
    );
  }

  fiterDisk(value, item) {
    if (
      Number(value?.extendInfo?.Capacity) >= Number(item?.extendInfo?.Capacity)
    ) {
      return !includes(this.cacheSelectedDisk, value[this.compareWithKey()]);
    }
  }

  setVaild() {
    this.disk$.next(
      !isEmpty(this.targetTableData?.data) &&
        every(this.targetTableData?.data, item => !isEmpty(item.targetDisk))
    );
  }

  diskChange(_, disk) {
    this.cacheSelectedDisk = reject(
      map(this.targetTableData?.data, 'targetDisk'),
      item => isEmpty(item)
    );
    each(this.targetTableData?.data, item => {
      if (item[this.compareWithKey()] === disk[this.compareWithKey()]) {
        return;
      }
      item.targetDiskOptions = filter(
        this.targetDisksOptions,
        value =>
          value[this.compareWithKey()] === item.targetDisk ||
          this.fiterDisk(value, item)
      );
    });
    this.setVaild();
  }

  getTargetPath() {
    return this.targerLocationInput;
  }

  getAllDisk() {
    return get(this.properties, 'volList', []).map(item => {
      set(item, 'extendInfo', JSON.parse(item.extendInfo));
      set(item, 'diskName', this.extractDiskName(item.name));
      return item;
    });
  }

  setExtParams(item) {
    assign(item, {
      type: item.extendInfo?.Type
    });
  }

  extractDiskName(path) {
    // 使用正则表达式匹配最后一个斜杠或反斜杠后面的内容
    const result = path.match(/[^\\/]+$/);
    return result ? result[0] : '';
  }

  getLeftTableData() {
    const allDisks = this.getAllDisk();
    let needRestoreDisks;
    if (
      this.resourceProperties.ext_parameters?.all_disk === 'True' ||
      this.resourceProperties.ext_parameters?.all_disk === 'true'
    ) {
      needRestoreDisks = allDisks;
    } else {
      needRestoreDisks = filter(allDisks, item =>
        includes(
          this.resourceProperties.ext_parameters?.disk_info,
          item[this.compareWithKey()]
        )
      );
    }
    each(needRestoreDisks, item => this.setExtParams(item));
    this.restoreTableData = {
      data: needRestoreDisks,
      total: size(needRestoreDisks)
    };
  }

  getParams() {
    delete this.targetParams.bootType;
    const params = this.targetParams?.requestParams;
    if (
      includes(
        [DataMap.Resource_Type.hyperVVm.value],
        this.rowCopy.resource_sub_type
      )
    ) {
      assign(params, {
        subObjects: map(this.targetTableData?.data, item => {
          const targetDisk = find(this.targetDisksOptions, {
            value: item.targetDisk
          });
          return JSON.stringify({
            uuid: item.uuid,
            extendInfo: {
              targetDisk: JSON.stringify(
                omit(targetDisk, ['value', 'key', 'label', 'isLeaf'])
              )
            }
          });
        })
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
