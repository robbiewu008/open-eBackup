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
import { TapeLibraryApiService } from 'app/shared/api/services/tape-library-api.service';
import { I18NService, DataMapService, DataMap } from 'app/shared';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { Observable, Observer, Subject } from 'rxjs';
import {
  assign,
  isEmpty,
  map,
  cloneDeep,
  includes,
  each,
  find,
  every,
  first
} from 'lodash';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-select-specific-drive',
  templateUrl: './batch-select-specific-slot.component.html',
  styleUrls: ['./batch-select-specific-slot.component.less']
})
export class BatchSelectSpecificSlotComponent implements OnInit {
  tapeList;
  node;
  optsConfig;
  chosen = [];

  tableConfig: TableConfig;
  tableData: TableData;
  selectionData = [];
  valid$ = new Subject<boolean>();
  SlotOptions = [];
  originalOptions = []; // 保存原始的数据
  originalIOOptions = [];
  originalCommonOptions = [];
  IOSlotOptions = [];
  CommonSlotOptions = [];
  TypeOptions = this.dataMapService.toArray('Tape_Slot_Type').filter(item => {
    item.isLeaf = true;
    return item.value !== DataMap.Tape_Slot_Type.invalid.value;
  });

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('driverSelectionExtraTpl', { static: true })
  driverSelectionExtraTpl: TemplateRef<any>;
  @ViewChild('typeSelectionExtraTpl', { static: true })
  typeSelectionExtraTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private dataMapService: DataMapService,
    private tapeLibraryApiService: TapeLibraryApiService,
    private virtualScroll: VirtualScrollService,
    private batchOperateService: BatchOperateService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.getSlotOptions();
    this.getSlotType();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'tapeLabel',
        name: this.i18n.get('system_archive_tape_labe_label')
      },
      {
        key: 'type',
        name: this.i18n.get('common_type_label'),
        cellRender: this.typeSelectionExtraTpl
      },
      {
        key: 'driverName',
        name: this.i18n.get('common_name_label'),
        cellRender: this.driverSelectionExtraTpl
      }
    ];
    this.typeSelectionExtraTpl.elementRef.nativeElement.type = first(
      this.TypeOptions
    )['value'];
    this.tableConfig = {
      table: {
        async: false,
        columns: cols,
        compareWith: 'driverSn',
        showLoading: false,
        colDisplayControl: false,
        virtualScroll: true,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple'
      }
    };
    this.tableData = {
      data: cloneDeep(this.tapeList),
      total: this.tapeList.length
    };
  }

  getSlotType() {
    const extParams = {
      tapeLibrarySn: this.tapeList[0].tapeLibrarySn,
      memberEsn: this.node?.remoteEsn,
      slotTypes: [
        DataMap.Tape_Slot_Type.import.value,
        DataMap.Tape_Slot_Type.other.value
      ]
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.tapeLibraryApiService.getTapeSlotsUsingGET(params),
      resource => {
        const commonType = [];
        const IOType = [];
        each(resource, item => {
          assign(item, {
            key: item.slotSn,
            value: item.slotName,
            label: item.slotName,
            isLeaf: true
          });
          if (item.type === DataMap.Tape_Slot_Type.other.value) {
            commonType.push(item);
          } else {
            IOType.push(item);
          }
        });
        this.originalIOOptions = IOType;
        this.originalCommonOptions = commonType;

        // 回显类型和名称
        this.getSlotInfo();
        each(this.tableData?.data, v => {
          const info = find(resource, item => v.targetSlot === item.slotName);
          v.type = info?.type;
          if (v.type === DataMap.Tape_Slot_Type.import.value) {
            this.IOSlotOptions = [...this.originalIOOptions];
          } else {
            this.CommonSlotOptions = [...this.originalCommonOptions];
          }
          this.getShowData(v);
        });
        this.valid$.next(
          every(this.tableData.data, item => !isEmpty(item.targetSlot))
        );
      },
      true,
      [],
      1
    );
  }

  getSlotInfo() {
    each(this.tableData?.data, item => {
      const extendInfo = item.extend?.split(',');
      const targetSlotInfo = find(
        extendInfo,
        item => item.indexOf('PreUsedSlot:') !== -1
      );
      const targetSlotName = targetSlotInfo?.split(':')[1];
      item.targetSlot = targetSlotName;
    });
  }

  getSlotOptions() {
    const params = {
      tapeLibrarySn: this.tapeList[0].tapeLibrarySn,
      memberEsn: this.node?.remoteEsn
    };
    this.tapeLibraryApiService.getTapeSlotsUsingGET(params).subscribe(res => {
      this.originalOptions = map(res.records, item => ({
        key: item?.slotName,
        value: item?.slotName,
        label: item?.slotName,
        isLeaf: true,
        slotType: DataMap.Tape_Slot_Type.other.value,
        ...item
      }));

      this.cdr.detectChanges();
    });
  }

  targetSlotChange(event?, data?, manualSelect?) {
    this.valid$.next(
      every(this.tableData.data, item => !isEmpty(item.targetSlot))
    );
    this.checkChosen();
  }

  typeChange(event, data) {
    data.targetSlot = '';
    if (data.type === DataMap.Tape_Slot_Type.import.value) {
      this.IOSlotOptions = [...this.originalIOOptions];
    } else {
      this.CommonSlotOptions = [...this.originalCommonOptions];
    }
    this.checkChosen();
    this.valid$.next(
      every(this.tableData.data, item => !isEmpty(item.targetSlot))
    );
  }

  checkChosen() {
    const ioChosen = [];
    const commonChosen = [];
    each(this.tableData.data, item => {
      if (!isEmpty(item.targetSlot)) {
        if (item.type === DataMap.Tape_Slot_Type.import.value) {
          ioChosen.push(item.targetSlot);
          find(this.IOSlotOptions, { value: item.targetSlot }).disabled = true;
        } else {
          commonChosen.push(item.targetSlot);
          find(this.CommonSlotOptions, {
            value: item.targetSlot
          }).disabled = true;
        }
      }
    });
    each(this.IOSlotOptions, item => {
      if (!includes(ioChosen, item.value)) item.disabled = false;
    });
    each(this.CommonSlotOptions, item => {
      if (!includes(commonChosen, item.value)) item.disabled = false;
    });
    this.CommonSlotOptions = [...this.CommonSlotOptions];
    this.IOSlotOptions = [...this.IOSlotOptions];
  }

  getShowData(item) {
    if (item.type === undefined) {
      return [];
    }
    return item.type === DataMap.Tape_Slot_Type.import.value
      ? this.IOSlotOptions
      : this.CommonSlotOptions;
  }

  clearAllDriver() {
    this.chosen = [];
    this.SlotOptions = [...this.originalOptions];
    each(this.tableData.data, item => {
      delete item.targetSlot;
    });
    this.targetSlotChange();
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.batchOperateService.selfGetResults(
        item =>
          this.tapeLibraryApiService.exportTapeUsingPUT({
            tapeLibrarySn: item.tapeLibrarySn,
            tapeLabel: item.tapeLabel,
            tapeExportRequest: { slotName: item.targetSlot },
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
