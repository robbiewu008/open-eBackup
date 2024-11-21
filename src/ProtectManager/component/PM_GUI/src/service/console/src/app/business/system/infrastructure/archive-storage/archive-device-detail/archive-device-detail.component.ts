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
import { Component, OnInit } from '@angular/core';
import { first, forEach } from 'lodash';
import { I18NService, DataMapService } from 'app/shared';
import {
  TableConfig,
  TableCols,
  Filters
} from 'app/shared/components/pro-table';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';

@Component({
  selector: 'aui-archive-device-detail',
  templateUrl: './archive-device-detail.component.html',
  styleUrls: ['./archive-device-detail.component.less']
})
export class ArchiveDeviceDetailComponent implements OnInit {
  active;
  libraryInfo;
  libraries = [];
  libraryData = [];
  tapeTableData = {};
  driveTableData = {};
  selectionData = [];
  activeIndex = 'drive';
  indentifyBtnDisable = true;
  tapeTableConfig: TableConfig;
  driveTableConfig: TableConfig;
  leftItems = [
    {
      key: 'name',
      label: this.i18n.get('common_name_label')
    },
    {
      key: 'serialNo',
      label: this.i18n.get('common_serial_number_label')
    },
    {
      key: 'vender',
      label: this.i18n.get('system_archive_vender_label')
    },
    {
      key: 'productID',
      label: this.i18n.get('system_archive_product_id_label')
    }
  ];
  rightItems = [
    {
      key: 'status',
      label: this.i18n.get('common_status_label')
    },
    {
      key: 'slotNo',
      label: this.i18n.get('system_archive_slot_num_label')
    },
    {
      key: 'driveNo',
      label: this.i18n.get('system_archive_drive_num_label')
    }
  ];

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    public virtualScroll: VirtualScrollService
  ) {}

  ngOnInit() {
    this.initTable();
    this.getLibraries();
  }

  initTable() {
    const driveCols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
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
        key: 'compression_status',
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
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'enable',
                label: this.i18n.get('common_enable_label'),
                onClick: data => {}
              },
              {
                id: 'disable',
                label: this.i18n.get('common_disable_label'),
                onClick: data => {}
              }
            ]
          }
        }
      }
    ];
    this.driveTableConfig = {
      table: {
        columns: driveCols,
        compareWith: 'id',
        showLoading: false,
        colDisplayControl: false,
        virtualScroll: true,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        fetchData: (filter: Filters, args) => {
          this.getDrives(filter, args);
        }
      }
    };

    const tapeCols: TableCols[] = [
      {
        key: 'tape_labe',
        name: this.i18n.get('system_archive_tape_labe_label')
      },
      {
        key: 'tape_set_name',
        name: this.i18n.get('system_archive_tape_set_name_label')
      },
      {
        key: 'location',
        name: this.i18n.get('common_location_label')
      },
      {
        key: 'write_status',
        name: this.i18n.get('system_archive_write_status_label'),
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
        key: 'status',
        name: this.i18n.get('common_status_label'),
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
        key: 'worm',
        name: this.i18n.get('system_tape_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Archive_Tape_Status')
        }
      },
      {
        key: 'time_stamp',
        name: this.i18n.get('common_time_stamp_label'),
        sort: true
      },
      {
        key: 'used_capcity',
        name: this.i18n.get('common_used_capcity_label')
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: [
              {
                id: 'import',
                label: this.i18n.get('common_import_label'),
                onClick: data => {}
              },
              {
                id: 'export',
                label: this.i18n.get('common_export_label'),
                onClick: data => {}
              },
              {
                id: 'delete',
                label: this.i18n.get('common_delete_label'),
                onClick: data => {}
              }
            ]
          }
        }
      }
    ];
    this.tapeTableConfig = {
      table: {
        compareWith: 'id',
        columns: tapeCols,
        virtualScroll: true,
        colDisplayControl: false,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        fetchData: (filter: Filters, args) => {
          this.getTapes(filter, args);
        },
        selectionChange: (renderSelection, selection) => {
          this.selectionData = selection;
        }
      }
    };
  }

  getLibraries() {
    this.libraryData = [
      {
        id: 1,
        name: 'Media Pool1',
        status: 0,
        slotNo: 11,
        serialNo: 1,
        vender: 'IBM1',
        driveNo: 1,
        productID: '11:11:11:11'
      },
      {
        id: 2,
        name: 'Media Pool2',
        status: 1,
        slotNo: 22,
        serialNo: 2,
        vender: 'IBM2',
        driveNo: 2,
        productID: '22:22:22:22'
      },
      {
        id: 3,
        name: 'Media Pool3',
        status: 1,
        slotNo: 33,
        serialNo: 3,
        vender: 'IBM3',
        driveNo: 3,
        productID: '33:33:33:33'
      }
    ];
    this.libraries = this.libraryData.filter(
      item => (
        (item.label = item.name),
        (item.onClick = event => this.getLibraryInfo(event.data.id))
      )
    );
    this.active = first(this.libraryData).id;
    this.getLibraryInfo(this.active);
    this.getDrives();
  }

  getLibraryInfo(id) {
    this.libraryInfo = this.libraryData.find(item => item.id === id);
    this.leftItems = forEach(
      this.leftItems,
      (item: any) => (item['value'] = this.libraryInfo[item.key])
    );
    this.rightItems = forEach(
      this.rightItems,
      (item: any) => (item['value'] = this.libraryInfo[item.key])
    );
  }

  getDrives(filter?: Filters, args?) {}

  getTapes(filter?: Filters, args?) {}

  indentify() {}

  libraryIndexChange(e) {
    if ('drive' === e) {
      this.getDrives();
    } else {
      this.getTapes();
    }
  }
}
