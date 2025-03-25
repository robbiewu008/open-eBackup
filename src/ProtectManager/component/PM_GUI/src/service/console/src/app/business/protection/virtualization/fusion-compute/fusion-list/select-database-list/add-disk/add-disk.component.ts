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
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  AppService,
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  isEmpty,
  isString,
  reject,
  size,
  take
} from 'lodash';
import { forkJoin, Observable, of } from 'rxjs';
import { mergeMap } from 'rxjs/operators';

@Component({
  selector: 'aui-add-disk-fc',
  templateUrl: './add-disk.component.html',
  styleUrls: ['./add-disk.component.less'],
  providers: [CapacityCalculateLabel]
})
export class AddDiskComponent implements OnInit {
  data;
  leftTableConfig: TableConfig;
  rightTableConfig: TableConfig;
  unitconst = CAPACITY_UNIT;

  totalTableData = {
    data: [],
    total: 0
  };
  selectionData = {
    data: [],
    total: 0
  };
  selectData;
  dataMap = DataMap;

  @ViewChild('operationTpl', { static: true }) operationTpl: TemplateRef<any>;
  @ViewChild('capacityTpl', { static: true }) capacityTpl: TemplateRef<any>;
  @ViewChild('totalDataTable', { static: false })
  totalDataTable: ProTableComponent;
  @ViewChild('selectDataTable', { static: false })
  selectDataTable: ProTableComponent;

  constructor(
    private modal: ModalRef,
    private appService: AppService,
    public i18n: I18NService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private cdr: ChangeDetectorRef,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.initData();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        name: this.i18n.get('common_name_label'),
        key: 'name',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        name: this.i18n.get('common_slot_label'),
        key: 'slot',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        name: this.i18n.get('common_datastore_label'),
        key: 'datastore',
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        name: this.i18n.get('protection_fc_disk_capacity_label'),
        key: 'capacity',
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.capacityTpl
      }
    ];

    const col = take(cols, 3);
    const cols1: TableCols[] = [
      {
        name: this.i18n.get('protection_fc_disk_capacity_label'),
        key: 'capacity',
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.operationTpl
      }
    ];

    this.leftTableConfig = {
      table: {
        async: false,
        compareWith: 'uuid',
        columns: cols,
        colDisplayControl: false,
        virtualScroll: true,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        scroll: { y: '640px' },
        selectionChange: selection => {
          this.selectData = selection;
          this.selectionData = {
            data: this.selectData,
            total: size(this.selectData)
          };
          this.disableOkBtn();
        },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        showTotal: true
      }
    };

    this.rightTableConfig = {
      table: {
        async: false,
        compareWith: 'uuid',
        columns: [...col, ...cols1],
        virtualScroll: true,
        colDisplayControl: false,
        scrollFixed: true,
        scroll: { y: '640px' },
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        showTotal: true
      }
    };
  }

  initData() {
    this.appUtilsService
      .getResourcesDetails(this.data, '', {}, {}, false)
      .subscribe(res => {
        const totalData = [...res];
        each(totalData, (item: any) => {
          assign(item, {
            slot: `${item?.extendInfo.pciType}(${item?.extendInfo.sequenceNum})`,
            datastore: item?.extendInfo?.datastoreName,
            capacity: item?.extendInfo?.quantityGB
          });
        });
        this.totalTableData = {
          data: totalData,
          total: size(totalData)
        };

        const selectData = !isEmpty(this.data.diskInfo)
          ? filter(this.totalTableData.data, item => {
              return find(
                this.data.diskInfo.map(curData =>
                  isString(curData) ? JSON.parse(curData) : curData
                ),
                { id: item.uuid }
              );
            })
          : [];

        if (selectData.length) {
          this.selectData = selectData;
          this.totalDataTable.setSelections(selectData);
          this.cdr.detectChanges();
        }

        this.selectionData = {
          data: selectData,
          total: size(selectData)
        };
        this.disableOkBtn();
      });
  }

  clearSelected() {
    this.selectionData = {
      data: [],
      total: 0
    };

    this.totalDataTable.setSelections([]);
    this.disableOkBtn();
  }

  removeSingle(item) {
    const newSelectData = reject(this.selectionData.data, (value: any) => {
      return item.uuid === value.uuid;
    });
    this.selectionData = {
      data: newSelectData,
      total: size(newSelectData)
    };

    this.totalDataTable.setSelections(newSelectData);
    this.disableOkBtn();
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = !size(this.selectionData.data);
  }

  onOK() {
    return cloneDeep(this.selectionData.data);
  }
}
