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
  ViewChild,
  ChangeDetectorRef,
  AfterViewInit
} from '@angular/core';
import {
  Filters,
  TableConfig,
  TableData,
  ProTableComponent,
  TableCols
} from 'app/shared/components/pro-table';
import { TapeLibraryApiService } from 'app/shared/api/services/tape-library-api.service';
import { I18NService, DataMapService, DataMap } from 'app/shared';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { Observable, Observer, Subject } from 'rxjs';
import { size, assign, isEmpty } from 'lodash';

@Component({
  selector: 'aui-select-specific-drive',
  templateUrl: './select-specific-drive.component.html',
  styleUrls: ['./select-specific-drive.component.less']
})
export class SelectSpecificDriveComponent implements OnInit, AfterViewInit {
  tapeLabel;
  tapeLibrarySn;
  optsConfig;
  tableConfig: TableConfig;
  tableData: TableData;
  selectionData = [];
  valid$ = new Subject<boolean>();
  node;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private tapeLibraryApiService: TapeLibraryApiService,
    private virtualScroll: VirtualScrollService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'status',
        name: this.i18n.get('system_running_satatus_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Archive_Tape_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Archive_Tape_Status')
        }
      },
      {
        key: 'compressionStatus',
        name: this.i18n.get('common_compressed_state_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Archive_Compression_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Archive_Compression_Status')
        }
      },
      {
        key: 'tapeLabel',
        name: this.i18n.get('system_archive_import_tape_labe_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];
    this.tableConfig = {
      table: {
        columns: cols,
        compareWith: 'driverSn',
        showLoading: false,
        colDisplayControl: false,
        rows: {
          selectionMode: 'single',
          selectionTrigger: 'selector',
          showSelector: true
        },
        virtualScroll: true,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        selectionChange: (renderSelection, selection) => {
          this.selectionData = selection;
          this.valid$.next(!!size(this.selectionData));
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple'
      }
    };
  }

  getData(filters?: Filters) {
    const params = {
      tapeLibrarySn: this.tapeLibrarySn,
      pageNo: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      memberEsn: this.node?.remoteEsn
    };
    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.name) {
        assign(params, { name: conditions.name });
      }
      if (conditions.status) {
        assign(params, { statuses: conditions.status });
      }
      if (conditions.compressionStatus) {
        assign(params, { compressionStatuses: conditions.compressionStatus });
      }
      if (conditions.tapeLabel) {
        assign(params, { tapeLabel: conditions.tapeLabel });
      }
    }
    this.tapeLibraryApiService.getTapeDrivesUsingGET(params).subscribe(res => {
      this.tableData = {
        data: res.records.filter(item =>
          item.tapeLabel ||
          item.status === DataMap.Archive_Tape_Status.disable.value
            ? assign(item, { disabled: true })
            : item
        ),
        total: res.totalCount
      };
      this.cdr.detectChanges();
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = {
        tapeLibrarySn: this.tapeLibrarySn,
        tapeLabel: this.tapeLabel,
        tapeImportRequest: { driveName: this.selectionData[0].name },
        memberEsn: this.node?.remoteEsn
      };
      this.tapeLibraryApiService.importTapeUsingPUT(params).subscribe(
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
