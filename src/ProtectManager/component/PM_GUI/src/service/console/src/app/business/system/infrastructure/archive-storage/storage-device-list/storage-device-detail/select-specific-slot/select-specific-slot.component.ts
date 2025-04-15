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
  ViewChild,
  ChangeDetectorRef
} from '@angular/core';
import {
  TableConfig,
  TableData,
  ProTableComponent,
  TableCols,
  Filters
} from 'app/shared/components/pro-table';
import { Subject, Observable, Observer } from 'rxjs';
import {
  I18NService,
  TapeLibraryApiService,
  DataMapService,
  DataMap,
  WarningMessageService
} from 'app/shared';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { size, isEmpty, assign } from 'lodash';

@Component({
  selector: 'aui-select-specific-slot',
  templateUrl: './select-specific-slot.component.html',
  styleUrls: ['./select-specific-slot.component.less']
})
export class SelectSpecificSlotComponent implements OnInit, AfterViewInit {
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
    private warningMessageService: WarningMessageService,
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
        key: 'slotName',
        name: this.i18n.get('system_archive_slot_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'type',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: false,
          options: this.dataMapService
            .toArray('Tape_Slot_Type')
            .filter(item => {
              return item.value !== DataMap.Tape_Slot_Type.invalid.value;
            })
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Tape_Slot_Type')
        }
      }
    ];
    this.tableConfig = {
      table: {
        columns: cols,
        compareWith: 'slotName',
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
      if (conditions.slotName) {
        assign(params, { slotName: conditions.slotName });
      }
      if (conditions.type) {
        assign(params, { slotTypes: conditions.type });
      }
    }

    this.tapeLibraryApiService.getTapeSlotsUsingGET(params).subscribe(res => {
      this.tableData = {
        data: res.records,
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
        tapeExportRequest: { slotName: this.selectionData[0].slotName },
        memberEsn: this.node?.remoteEsn
      };
      if (this.selectionData[0].type === DataMap.Tape_Slot_Type.import.value) {
        this.warningMessageService.create({
          content: this.i18n.get('system_slot_export_label', [
            this.selectionData[0].slotName
          ]),
          onOK: () => {
            this.tapeLibraryApiService.exportTapeUsingPUT(params).subscribe(
              res => {
                observer.next();
                observer.complete();
              },
              err => {
                observer.error(err);
                observer.complete();
              }
            );
          },
          onCancel: () => {
            observer.error('');
            observer.complete();
          }
        });
        return;
      }
      this.tapeLibraryApiService.exportTapeUsingPUT(params).subscribe(
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
