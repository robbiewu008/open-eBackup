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
  TemplateRef
} from '@angular/core';
import {
  TableConfig,
  TableData,
  ProTableComponent,
  TableCols
} from 'app/shared/components/pro-table';
import {
  I18NService,
  DataMapService,
  DataMap,
  TapeLibraryApiService
} from 'app/shared';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { Observable, Observer, Subject } from 'rxjs';
import {
  size,
  assign,
  isEmpty,
  filter,
  map,
  get,
  cloneDeep,
  includes,
  each,
  reject,
  find,
  every
} from 'lodash';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';

@Component({
  selector: 'aui-select-specific-drive',
  templateUrl: './batch-select-specific-drive.component.html',
  styleUrls: ['./batch-select-specific-drive.component.less']
})
export class BatchSelectSpecificDriveComponent implements OnInit {
  tapeList;
  node;
  optsConfig;
  chosen = [];

  tableConfig: TableConfig;
  tableData: TableData;
  selectionData = [];
  valid$ = new Subject<boolean>();
  driveOptions = [];
  originalOptions = []; // 保存原始的数据

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('driverSelectionExtraTpl', { static: true })
  driverSelectionExtraTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private tapeLibraryApiService: TapeLibraryApiService,
    private virtualScroll: VirtualScrollService,
    private batchOperateService: BatchOperateService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.getDriveOptions();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'tapeLabel',
        name: this.i18n.get('system_archive_tape_labe_label')
      },
      {
        key: 'driverName',
        name: this.i18n.get('common_name_label'),
        cellRender: this.driverSelectionExtraTpl
      },
      {
        key: 'driverStatus',
        name: this.i18n.get('system_tape_running_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Archive_Tape_Status')
        }
      },
      {
        key: 'driverCompressionStatus',
        name: this.i18n.get('system_tape_compression_status_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Archive_Compression_Status')
        }
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        columns: cols,
        compareWith: 'driverSn',
        showLoading: false,
        colDisplayControl: false,
        virtualScroll: true,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
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
    this.tableData = {
      data: this.tapeList,
      total: this.tapeList.length
    };
  }

  getDriveOptions() {
    const params = {
      tapeLibrarySn: this.tapeList[0].tapeLibrarySn,
      memberEsn: this.node?.remoteEsn
    };
    this.tapeLibraryApiService.getTapeDrivesUsingGET(params).subscribe(res => {
      const driveFilter: any = filter(res.records, item => {
        return item.tapeLabel ||
          item.status === DataMap.Archive_Tape_Status.disable.value
          ? assign(item, { disabled: true })
          : item;
      });

      this.originalOptions = map(driveFilter, item => ({
        key: item?.name,
        value: item?.name,
        label: item?.name,
        isLeaf: true,
        ...item
      }));
      this.driveOptions = cloneDeep(this.originalOptions);

      for (let index = 0; index < size(this.tableData.data); index++) {
        if (index + 1 > size(this.originalOptions)) {
          return false;
        }
        if (this.originalOptions[index]?.disabled) {
          continue;
        }
        const target = this.originalOptions[index];
        const {
          value: targetDriver,
          status: driverStatus,
          compressionStatus: driverCompressionStatus
        } = target;
        assign(this.tableData.data[index], {
          targetDriver,
          driverStatus,
          driverCompressionStatus
        });
        this.targetDriverChange('', target, false);
      }
      this.cdr.detectChanges();
    });
  }

  targetDriverChange(event, data, manualSelect) {
    const cloneDriverOptions = cloneDeep(this.originalOptions);
    this.chosen = reject(map(this.tableData?.data, 'targetDriver'), item =>
      isEmpty(item)
    );

    each(cloneDriverOptions, item => {
      if (includes(this.chosen, item.value)) {
        item.disabled = true;
      }
    });
    this.driveOptions = [...cloneDriverOptions];
    if (manualSelect) {
      assign(data, {
        driverStatus: get(
          find(this.originalOptions, { label: data.targetDriver }),
          'status',
          null
        ),
        driverCompressionStatus: get(
          find(this.originalOptions, { label: data.targetDriver }),
          'compressionStatus',
          null
        )
      });
    }
    this.valid$.next(
      every(this.tableData.data, item => !isEmpty(item.targetDriver))
    );
  }

  clearAllDriver() {
    this.chosen = [];
    this.driveOptions = [...this.originalOptions];
    each(this.tableData.data, item => {
      delete item.targetDriver;
      delete item.driverStatus;
      delete item.driverCompressionStatus;
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.batchOperateService.selfGetResults(
        item =>
          this.tapeLibraryApiService.importTapeUsingPUT({
            tapeLibrarySn: item.tapeLibrarySn,
            tapeLabel: item.tapeLabel,
            tapeImportRequest: { driveName: item.targetDriver },
            memberEsn: this.node?.remoteEsn,
            akDoException: false,
            akOperationTips: false,
            akLoading: false
          }),
        map(cloneDeep(this.tableData.data), item => {
          return assign(item, {
            name: item.tapeLabel,
            isAsyn: false
          });
        }),
        () => {
          observer.next();
          observer.complete();
        }
      );
    });
  }
}
