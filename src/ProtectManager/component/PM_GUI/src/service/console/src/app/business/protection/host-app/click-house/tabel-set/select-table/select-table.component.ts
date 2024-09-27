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
  OnInit,
  AfterViewInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedEnvironmentApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { cloneDeep, each, includes, isNil, map, reject, size } from 'lodash';

@Component({
  selector: 'aui-select-table',
  templateUrl: './select-table.component.html',
  styleUrls: ['./select-table.component.less']
})
export class SelectTableComponent implements OnInit, AfterViewInit {
  item;
  data;
  selectedTableData;
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
  selectData = [];
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
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initConfig();
  }
  ngAfterViewInit(): void {
    this.totalDataTable.fetchData();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      }
    ];

    const cols1: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        cellRender: this.operationTpl
      }
    ];

    this.leftTableConfig = {
      table: {
        compareWith: 'name',
        columns: cols,
        colDisplayControl: false,
        virtualScroll: true,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        scroll: { y: '480px' },
        selectionChange: selection => {
          this.setSelection(selection).disableOkBtn();
        },
        trackByFn: (_, item) => {
          return item.name;
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
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
        compareWith: 'name',
        columns: cols1,
        virtualScroll: true,
        colDisplayControl: false,
        scrollFixed: true,
        scroll: { y: '480px' },
        trackByFn: (index, item) => {
          return item.name;
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
  getData(filter, args) {
    this._getData(filter?.paginator.pageIndex + 1);
  }

  _getData(pageNo = CommonConsts.PAGE_START + 1) {
    const params = {
      pageNo: pageNo,
      pageSize: CommonConsts.PAGE_SIZE,
      envId: this.data?.clusterId,
      parentId: this.data?.parentId,
      resourceType: DataMap.Resource_Type.ClickHouse.value
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        this.totalTableData = {
          data: map(res.records, item => {
            const disabledItem = item => {
              // 编辑时禁用场景
              if (
                !isNil(item.parentUuid) &&
                !isNil(this.item) &&
                item.parentUuid !== this.item.uuid
              ) {
                return true;
              }
              // 创建时禁用场景
              if (isNil(this.item) && !isNil(item.parentUuid)) {
                return true;
              }
              return false;
            };
            return {
              ...item,
              disabled: disabledItem(item)
            };
          }),
          total: res.totalCount
        };
        const _selection = res.records.filter(item => {
          if (!isNil(this.selectedTableData.find(i => i.name === item.name))) {
            return true;
          }

          return false;
        });

        this.setSelection([...this.selectData, ..._selection]).disableOkBtn();
      });
  }

  clearSelected() {
    this.setSelection([]).disableOkBtn();
  }

  removeSingle(item) {
    const newSelectData = reject(this.selectionData.data, (value: any) => {
      return item.name === value.name;
    });

    this.setSelection(newSelectData).disableOkBtn();
  }

  private setSelection(selection) {
    const _selection = this._uniqueSelection(selection);
    this.totalDataTable.setSelections(_selection);
    this.selectData = _selection;
    this.selectionData = {
      data: _selection,
      total: size(_selection)
    };
    return this;
  }

  private _uniqueSelection(selection: Array<any>) {
    const result = [];
    each(selection, item => {
      if (!result.find(i => i.name === item.name)) {
        result.push(item);
      }
    });
    return result;
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = !size(this.selectionData.data);
  }

  onOK() {
    return cloneDeep(this.selectionData.data.map(item => item.id));
  }

  get diffSelection() {
    return this.selectedTableData
      .filter(
        i =>
          !includes(
            this.selectData.map(j => j.name),
            i.name
          )
      )
      .map(i => i.name)
      .map(i => {
        return { uuid: this.totalTableData.data.find(j => j.name === i).uuid };
      });
  }
}
