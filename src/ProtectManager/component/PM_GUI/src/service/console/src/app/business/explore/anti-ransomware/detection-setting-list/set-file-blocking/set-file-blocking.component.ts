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
  ChangeDetectorRef,
  Component,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  DataMapService,
  I18NService,
  FileExtensionFilterManagementService,
  CommonConsts,
  DataMap
} from 'app/shared';
import {
  TableConfig,
  TableData,
  ProTableComponent,
  TableCols,
  Filters
} from 'app/shared/components/pro-table';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { size, map, assign, isEmpty, isNumber, includes } from 'lodash';
import { Subject, Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-set-file-blocking',
  templateUrl: './set-file-blocking.component.html',
  styleUrls: ['./set-file-blocking.component.less']
})
export class SetFileBlockingComponent implements OnInit, AfterViewInit {
  isDetail = false;
  vstoreId;
  optsConfig;
  tableConfig: TableConfig;
  tableData: TableData;
  selectionData = [];
  valid$ = new Subject<boolean>();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private virtualScroll: VirtualScrollService,
    private fileExtensionFilterManagementService: FileExtensionFilterManagementService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
    if (!this.isDetail) {
      this.setSelection();
    }
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'fileExtensionName',
        name: this.i18n.get('explore_file_extension_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'fileExtensionType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('File_Extension_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('File_Extension_Type')
        }
      },
      {
        key: 'updateTime',
        name: this.i18n.get('explore_add_tiem_label'),
        sort: true
      }
    ];
    if (this.isDetail) {
      cols.splice(2, 0, {
        key: 'importStatus',
        name: this.i18n.get('explore_file_extension_associate_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService
            .toArray('File_Extension_Import_Status')
            .filter(
              item =>
                !includes(
                  [DataMap.File_Extension_Import_Status.deleteError.value],
                  item.value
                )
            )
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('File_Extension_Import_Status')
        }
      });
    }
    this.tableConfig = {
      table: {
        columns: cols,
        compareWith: 'fileExtensionName',
        showLoading: false,
        colDisplayControl: false,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        virtualScroll: true,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple'
      }
    };
    if (this.isDetail) {
      delete this.tableConfig.table.rows;
      delete this.tableConfig.table.selectionChange;
    }
  }

  getData(filters?: Filters) {
    const params = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize
    };

    if (this.isDetail) {
      assign(params, { vstoreId: this.vstoreId });
    }
    if (!!size(filters.orders)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }
    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.fileExtensionName) {
        assign(params, { fileExtensionName: conditions.fileExtensionName });
      }
      if (conditions.fileExtensionType) {
        assign(params, { fileExtensionType: conditions.fileExtensionType });
      }
      if (conditions.importStatus) {
        assign(params, { importStatus: conditions.importStatus });
      }
    }
    this.fileExtensionFilterManagementService
      .getFileExtensionFilterUsingGET(params)
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  setSelection(recordsTemp?, startPage?) {
    const params = {
      pageNum: startPage || CommonConsts.PAGE_START + 1,
      pageSize: CommonConsts.PAGE_SIZE_OPTIONS[2],
      vstoreId: this.vstoreId
    };
    this.fileExtensionFilterManagementService
      .getFileExtensionFilterUsingGET(params)
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START + 1;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage ===
            Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE_OPTIONS[2]) + 1 ||
          res.totalCount === 0
        ) {
          this.dataTable.setSelections(recordsTemp);
          this.selectionData = recordsTemp;
          this.valid$.next(!!size(recordsTemp));
          return;
        }
        this.setSelection(recordsTemp, startPage);
      });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        modifyUserSuffixRequest: {
          vstoreId: this.vstoreId,
          extensions: map(this.selectionData, 'fileExtensionName')
        }
      };
      this.fileExtensionFilterManagementService
        .updateFileExtensionFilterUsingPUT(params)
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
}
