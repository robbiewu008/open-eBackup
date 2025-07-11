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
import { ModalRef } from '@iux/live';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  isOpenstackSystemDisk
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import {
  assign,
  cloneDeep,
  each,
  filter,
  get,
  includes,
  isEmpty,
  reject,
  size
} from 'lodash';

@Component({
  selector: 'aui-add-disk-openstack',
  templateUrl: './add-disk.component.html',
  styleUrls: ['./add-disk.component.less']
})
export class AddDiskComponent implements OnInit {
  data;
  leftTableConfig: TableConfig;
  rightTableConfig: TableConfig;

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
  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('totalDataTable', { static: false })
  totalDataTable: ProTableComponent;
  @ViewChild('selectDataTable', { static: false })
  selectDataTable: ProTableComponent;

  constructor(
    private modal: ModalRef,
    public i18n: I18NService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.initData();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'nameId',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'diskType',
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
      }
    ];

    const lcols: TableCols[] = [
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.sizeTpl
      }
    ];

    const rcols: TableCols[] = [
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        cellRender: this.operationTpl
      }
    ];

    this.leftTableConfig = {
      table: {
        async: false,
        compareWith: 'id',
        columns: [...cols, ...lcols],
        colDisplayControl: false,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        selectionChange: selection => {
          this.selectData = selection;
          this.selectionData = {
            data: this.selectData,
            total: size(this.selectData)
          };
          this.disableOkBtn();
        },
        trackByFn: (_, item) => {
          return item.id;
        }
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        showTotal: true
      }
    };

    this.rightTableConfig = {
      table: {
        async: false,
        compareWith: 'id',
        columns: [...cols, ...rcols],
        colDisplayControl: false,
        scrollFixed: true,
        trackByFn: (_, item) => {
          return item.id;
        }
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        showTotal: true
      }
    };
  }

  initData() {
    const allDisk = JSON.parse(get(this.data, ['extendInfo', 'volInfo']));
    // 磁盘名称可能没有，用uuid区分
    each(allDisk, item => {
      assign(item, {
        nameId: `${item.name || '--'}(${item.id})`,
        diskType: isOpenstackSystemDisk(item.device, item.bootable)
      });
    });
    setTimeout(() => {
      this.totalTableData = {
        data: cloneDeep(allDisk),
        total: size(allDisk)
      };

      const showData = !isEmpty(this.data.diskInfo)
        ? filter(allDisk, v => {
            return includes(this.data.diskInfo, v.id);
          })
        : !isEmpty(this.data.protectedObject)
        ? filter(allDisk, item => {
            return includes(
              this.data.protectedObject?.extParameters?.disk_info,
              item.id
            );
          })
        : [];

      if (showData.length) {
        this.selectData = showData;
        this.totalDataTable.setSelections(showData);
      }
      this.selectionData = {
        data: showData,
        total: size(showData)
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
      return item.id === value.id;
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
    return cloneDeep(this.selectionData.data.map(item => item.id));
  }
}
