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
  ChangeDetectorRef,
  ChangeDetectionStrategy,
  TemplateRef
} from '@angular/core';
import {
  Filters,
  TableCols,
  TableConfig,
  ProTableComponent,
  TableData
} from 'app/shared/components/pro-table';
import {
  forEach,
  assign,
  isEmpty,
  isUndefined,
  includes,
  upperCase,
  size,
  cloneDeep,
  map,
  some
} from 'lodash';
import {
  I18NService,
  DataMapService,
  WarningMessageService,
  CookieService
} from 'app/shared/services';
import {
  TapeLibraryApiService,
  MODAL_COMMON,
  DataMap,
  CommonConsts,
  TapeLocation,
  CAPACITY_UNIT,
  getAccessibleViewList,
  OperateItems,
  RoleType,
  isRBACDPAdmin
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { SetBlockSizeComponent } from './set-block-size/set-block-size.component';
import { SelectSpecificDriveComponent } from './select-specific-drive/select-specific-drive.component';
import { SelectSpecificSlotComponent } from './select-specific-slot/select-specific-slot.component';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { BatchSelectSpecificDriveComponent } from './batch-select-specific-drive/batch-select-specific-drive.component';
import { BatchSelectSpecificSlotComponent } from './batch-specific-slot/batch-select-specific-slot.component';
import { ProButton } from 'app/shared/components/pro-button/interface';

@Component({
  selector: 'aui-storage-device-detail',
  templateUrl: './storage-device-detail.component.html',
  styleUrls: ['./storage-device-detail.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class StorageDeviceDetailComponent implements OnInit, AfterViewInit {
  item;
  node;
  optsConfig;
  dataMap = DataMap;
  tapeTableData: TableData;
  driveTableData: TableData;
  selectionData = [];
  activeIndex = 'drive';
  unitconst = CAPACITY_UNIT;
  tapeTableConfig: TableConfig;
  driveTableConfig: TableConfig;
  leftItems1 = [
    {
      key: 'name',
      label: this.i18n.get('common_name_label')
    },
    {
      key: 'status',
      label: this.i18n.get('common_status_label')
    }
  ];
  leftItems2 = [
    {
      key: 'serialNo',
      label: this.i18n.get('common_serial_number_label')
    },
    {
      key: 'vendor',
      label: this.i18n.get('system_archive_vender_label')
    }
  ];
  rightItems1 = [
    {
      key: 'productId',
      label: this.i18n.get('system_archive_product_id_label')
    },
    {
      key: 'slotNumber',
      label: this.i18n.get('system_archive_slot_num_label')
    }
  ];
  rightItems2 = [
    {
      key: 'driveNumber',
      label: this.i18n.get('system_archive_drive_num_label')
    },
    {
      key: 'controllerName',
      label: this.i18n.get('common_home_node_label')
    }
  ];
  showArchiveDrive = true;
  @ViewChild('driveTable', { static: false }) driveTable: ProTableComponent;
  @ViewChild('tapeTable', { static: false }) tapeTable: ProTableComponent;
  @ViewChild('capacityTpl', { static: true }) capacityTpl: TemplateRef<any>;
  @ViewChild('blockSizeTpl', { static: true }) blockSizeTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private batchOperateService: BatchOperateService,
    private tapeLibraryApiService: TapeLibraryApiService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    this.getArchiveDrive();
    this.initTable();
    this.getLibraryInfo();
    this.initConfig();
  }

  getArchiveDrive() {
    this.showArchiveDrive = !isRBACDPAdmin(this.cookieService.role);
    if (!this.showArchiveDrive) {
      this.activeIndex = 'tape';
    }
  }

  ngAfterViewInit() {
    if (this.driveTable) {
      this.driveTable.fetchData();
    }
    this.tapeTable.fetchData();
  }

  initTable() {
    const driveCols: TableCols[] = [
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
          showCheckAll: false,
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
      },
      {
        key: 'driverSn',
        name: this.i18n.get('common_serial_number_label')
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
                label: this.i18n.get('system_enable_tape_label'),
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.ScanTapeLibrary
                  ];
                },
                disableCheck: data => {
                  return (
                    data[0].status ===
                      DataMap.Archive_Tape_Status.enable.value ||
                    this.item.status === DataMap.Media_Tape_Status.offline.value
                  );
                },
                onClick: data => {
                  this.enable(data);
                }
              },
              {
                id: 'disable',
                label: this.i18n.get('system_disable_tape_label'),
                disableCheck: data => {
                  return (
                    !isEmpty(data[0].tapeLabel) ||
                    data[0].status ===
                      DataMap.Archive_Tape_Status.disable.value ||
                    this.item.status === DataMap.Media_Tape_Status.offline.value
                  );
                },
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.ScanTapeLibrary
                  ];
                },
                onClick: data => {
                  this.disable(data);
                }
              }
            ]
          }
        }
      }
    ];
    this.driveTableConfig = {
      table: {
        size: 'small',
        autoPolling: CommonConsts.TIME_INTERVAL,
        columns: driveCols,
        compareWith: 'id',
        showLoading: false,
        colDisplayControl: false,
        virtualScroll: true,
        scrollFixed: true,
        virtualItemHeight: 24,
        scroll: { y: '290px' },
        fetchData: (filter: Filters, args) => {
          this.getDrives(filter, args);
        }
      },
      pagination: {
        mode: 'simple',
        winTablePagination: true,
        pageSizeOptions: [CommonConsts.PAGE_SIZE_SMALL],
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };

    const tapeCols: TableCols[] = [
      {
        key: 'tapeLabel',
        name: this.i18n.get('system_archive_tape_labe_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'mediaSetName',
        name: this.i18n.get('system_archive_tape_set_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'location',
        name: this.i18n.get('common_location_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'writeStatus',
        name: this.i18n.get('system_archive_write_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: false,
          options: this.dataMapService.toArray('Tape_Write_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Tape_Write_Status')
        }
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: false,
          options: this.dataMapService.toArray('Library_Tape_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Library_Tape_Status')
        }
      },
      {
        key: 'worm',
        name: this.i18n.get('system_tape_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: false,
          options: this.dataMapService
            .toArray('Media_Pool_Type')
            .filter(v => DataMap.Media_Pool_Type.worm.value !== v.value)
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Media_Pool_Type')
        }
      },
      {
        key: 'lastWriteTime',
        name: this.i18n.get('system_last_write_time_label'),
        sort: true
      },
      {
        key: 'usedCapacity',
        name: this.i18n.get('common_used_capcity_label'),
        thAlign: 'right',
        cellRender: this.capacityTpl
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
                id: 'import',
                label: this.i18n.get('system_tape_load_label'),
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.ScanTapeLibrary
                  ];
                },
                disableCheck: data => {
                  return (
                    this.item.status ===
                      DataMap.Media_Tape_Status.offline.value ||
                    data[0].status !==
                      this.dataMap.Library_Tape_Status.inLibrary.value ||
                    data[0].location.indexOf(TapeLocation.Driver) !== -1
                  );
                },
                onClick: data => {
                  this.improt(data);
                }
              },
              {
                id: 'export',
                label: this.i18n.get('system_tape_unload_label'),
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.ScanTapeLibrary
                  ];
                },
                disableCheck: data => {
                  return (
                    this.item.status ===
                      DataMap.Media_Tape_Status.offline.value ||
                    !includes(
                      [this.dataMap.Library_Tape_Status.ready.value],
                      data[0].status
                    ) ||
                    data[0].location.indexOf(TapeLocation.Slot) !== -1
                  );
                },
                onClick: data => {
                  this.export(data);
                }
              },
              {
                id: 'delete',
                label: this.i18n.get('common_delete_label'),
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.ScanTapeLibrary
                  ];
                },
                disableCheck: data => {
                  return (
                    this.item.status ===
                      DataMap.Media_Tape_Status.offline.value ||
                    data[0].status !==
                      this.dataMap.Library_Tape_Status.notInLibrary.value
                  );
                },
                onClick: data => {
                  this.delete(data);
                }
              },
              {
                id: 'indentify',
                label: this.i18n.get('system_archive_indentify_label'),
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.ScanTapeLibrary
                  ];
                },
                disableCheck: data => {
                  return (
                    this.item.status ===
                      DataMap.Media_Tape_Status.offline.value ||
                    data[0].status !==
                      this.dataMap.Library_Tape_Status.inLibrary.value ||
                    data[0].writeStatus ===
                      this.dataMap.Tape_Write_Status.error.value
                  );
                },
                onClick: data => {
                  this.indentifyTape(data);
                }
              },
              {
                id: 'markEmpty',
                label: this.i18n.get('system_tape_marking_empty_label'),
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.ScanTapeLibrary
                  ];
                },
                disableCheck: data => {
                  return (
                    this.item.status ===
                      DataMap.Media_Tape_Status.offline.value ||
                    data[0].status !==
                      this.dataMap.Library_Tape_Status.inLibrary.value ||
                    data[0].worm === this.dataMap.Media_Pool_Type.worm.value ||
                    data[0].writeStatus ===
                      this.dataMap.Tape_Write_Status.error.value
                  );
                },
                onClick: data => {
                  this.markEmpty(data);
                }
              },
              {
                id: 'earse',
                label: this.i18n.get('system_tape_erase_label'),
                displayCheck: () => {
                  return getAccessibleViewList(this.cookieService.role)[
                    OperateItems.ScanTapeLibrary
                  ];
                },
                disableCheck: data => {
                  return (
                    this.item.status ===
                      DataMap.Media_Tape_Status.offline.value ||
                    data[0].status !==
                      this.dataMap.Library_Tape_Status.inLibrary.value ||
                    includes(
                      [this.dataMap.Tape_Write_Status.error.value],
                      data[0].writeStatus
                    ) ||
                    data[0].worm === this.dataMap.Media_Pool_Type.worm.value ||
                    !!size(data[0].lockKey)
                  );
                },
                onClick: data => {
                  this.earse(data);
                }
              }
            ]
          }
        }
      }
    ];
    this.tapeTableConfig = {
      table: {
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        size: 'small',
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'tapeLabel',
        columns: tapeCols,
        virtualScroll: false,
        colDisplayControl: false,
        scrollFixed: true,
        virtualItemHeight: 24,
        scroll: { y: '290px' },
        fetchData: (filter: Filters, args) => {
          this.getTapes(filter, args);
        },
        selectionChange: (renderSelection, selection) => {
          this.selectionData = selection;
        }
      },
      pagination: {
        mode: 'simple',
        winTablePagination: true,
        pageSizeOptions: [CommonConsts.PAGE_SIZE_SMALL],
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'batchLoad',
        label: this.i18n.get('system_tape_load_label'),
        displayCheck: () => {
          return getAccessibleViewList(this.cookieService.role)[
            OperateItems.CreateArchiveStorage
          ];
        },
        disableCheck: data => {
          return (
            !size(data) ||
            some(data, selectItem => {
              return (
                this.item.status === DataMap.Media_Tape_Status.offline.value ||
                selectItem.status !==
                  this.dataMap.Library_Tape_Status.inLibrary.value ||
                selectItem.location.indexOf(TapeLocation.Driver) !== -1
              );
            })
          );
        },
        onClick: () => {
          this.batchLoad(this.selectionData);
        }
      },
      {
        id: 'batchUnLoad',
        label: this.i18n.get('system_tape_unload_label'),
        displayCheck: () => {
          return getAccessibleViewList(this.cookieService.role)[
            OperateItems.DeletingArchiveStorage
          ];
        },
        disableCheck: data => {
          return (
            !size(data) ||
            some(data, selectItem => {
              return (
                this.item.status === DataMap.Media_Tape_Status.offline.value ||
                !includes(
                  [this.dataMap.Library_Tape_Status.ready.value],
                  selectItem.status
                ) ||
                selectItem.location.indexOf(TapeLocation.Slot) !== -1
              );
            })
          );
        },
        onClick: data => {
          this.batchunLoad(this.selectionData);
        }
      },
      {
        id: 'batchIdentifyTape',
        label: this.i18n.get('system_archive_indentify_label'),
        onClick: () => {
          this.batchIdentifyTape();
        },
        disableCheck: data => {
          return !size(this.selectionData);
        }
      }
    ];
    this.optsConfig = opts;
  }
  getLibraryInfo() {
    this.leftItems1 = forEach(
      this.leftItems1,
      (item: any) => (item['value'] = this.item[item.key])
    );
    this.leftItems2 = forEach(
      this.leftItems2,
      (item: any) => (item['value'] = this.item[item.key])
    );
    this.rightItems1 = forEach(
      this.rightItems1,
      (item: any) => (item['value'] = this.item[item.key])
    );
    this.rightItems2 = forEach(
      this.rightItems2,
      (item: any) => (item['value'] = this.item[item.key])
    );
  }

  getDrives(filters, args) {
    const params = {
      tapeLibrarySn: this.item.serialNo,
      pageSize: filters.paginator.pageSize,
      pageNo: filters.paginator.pageIndex + 1,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true,
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
      this.driveTableData = {
        data: res.records,
        total: res.totalCount
      };
      this.cdr.detectChanges();
    });
  }

  getTapes(filters, args) {
    const params = {
      tapeLibrarySn: this.item.serialNo,
      pageSize: filters.paginator.pageSize,
      pageNo: filters.paginator.pageIndex + 1,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true,
      memberEsn: this.node?.remoteEsn
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.tapeLabel) {
        assign(params, { tapeLabel: conditions.tapeLabel });
      }
      if (conditions.mediaSetName) {
        assign(params, { mediaSetName: conditions.mediaSetName });
      }
      if (conditions.location) {
        assign(params, { location: conditions.location });
      }
      if (conditions.writeStatus) {
        assign(params, { writeStatuses: conditions.writeStatus });
      }
      if (conditions.status) {
        assign(params, { statuses: conditions.status });
      }
      if (conditions.worm) {
        assign(params, { worms: conditions.worm });
      }
    }

    if (filters.sort.key === 'lastWriteTime') {
      assign(params, {
        orderType: upperCase(filters.sort.direction),
        orderBy: 'LAST_WRITE_TIME'
      });
    }

    this.tapeLibraryApiService
      .getLibraryTapesUsingGET(params)
      .subscribe(res => {
        this.tapeTableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  enable(data) {
    this.tapeLibraryApiService
      .modifyTapeDriveUsingPUT({
        tapeLibrarySn: this.item.serialNo,
        tapeDriveModifyRequest: {
          blockSize: data[0].blockSize,
          compressionStatus: data[0].compressionStatus,
          status: 'ENABLE'
        },
        driverSn: data[0].driverSn,
        memberEsn: this.node?.remoteEsn
      })
      .subscribe(res => {
        this.driveTable.fetchData();
      });
  }

  disable(data) {
    this.tapeLibraryApiService
      .modifyTapeDriveUsingPUT({
        tapeLibrarySn: this.item.serialNo,
        tapeDriveModifyRequest: {
          blockSize: data[0].blockSize,
          compressionStatus: data[0].compressionStatus,
          status: 'DISABLE'
        },
        driverSn: data[0].driverSn,
        memberEsn: this.node?.remoteEsn
      })
      .subscribe(res => {
        this.driveTable.fetchData();
      });
  }

  batchLoad(data) {
    const lvComponentParamsTest = {
      node: this.node,
      tapeLibrarySn: this.item.serialNo,
      tapeLabel: data[0].tapeLabel
    };
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.largeWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('system_tape_load_label'),
        lvComponentParams: {
          node: this.node,
          tapeList: this.selectionData
        },
        lvContent: BatchSelectSpecificDriveComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as BatchSelectSpecificDriveComponent;
          content.valid$.subscribe(res => {
            modal.lvOkDisabled = !res;
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as BatchSelectSpecificDriveComponent;
            content.onOK().subscribe(
              res => {
                resolve(true);
                this.selectionData = [];
                this.tapeTable.setSelections([]);
                this.tapeTable.fetchData();
              },
              error => resolve(false)
            );
          });
        }
      }
    });
  }

  batchunLoad(data) {
    const lvComponentParamsTest = {
      node: this.node,
      tapeLibrarySn: this.item.serialNo,
      tapeLabel: data[0].tapeLabel
    };
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.largeWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('system_tape_unload_label'),
        lvComponentParams: {
          node: this.node,
          tapeList: this.selectionData
        },
        lvContent: BatchSelectSpecificSlotComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as BatchSelectSpecificSlotComponent;
          content.valid$.subscribe(res => {
            modal.lvOkDisabled = !res;
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as BatchSelectSpecificSlotComponent;
            content.onOK().subscribe(
              res => {
                resolve(true);
                this.selectionData = [];
                this.tapeTable.setSelections([]);
                this.tapeTable.fetchData();
              },
              error => resolve(false)
            );
          });
        }
      }
    });
  }

  batchIdentifyTape() {
    if (size(this.selectionData) === 1) {
      this.indentifyTape(this.selectionData);
      return;
    }
    this.batchOperateService.selfGetResults(
      item => {
        return this.tapeLibraryApiService.identifyTapeUsingPUT({
          tapeLibrarySn: item.tapeLibrarySn,
          tapeLabel: item.tapeLabel,
          akDoException: false,
          akOperationTips: false,
          akLoading: false
        });
      },
      map(cloneDeep(this.selectionData), item => {
        return assign(item, {
          name: item.tapeLabel,
          isAsyn: false
        });
      }),
      () => {
        this.selectionData = [];
        this.tapeTable.setSelections([]);
        this.tapeTable.fetchData();
      }
    );
  }

  compressionEnable(data) {
    this.tapeLibraryApiService
      .modifyTapeDriveUsingPUT({
        tapeLibrarySn: this.item.serialNo,
        tapeDriveModifyRequest: {
          blockSize: data[0].blockSize,
          compressionStatus: 'ENABLE',
          status: data[0].status
        },
        driverSn: data[0].driverSn,
        memberEsn: this.node?.remoteEsn
      })
      .subscribe(res => {
        this.driveTable.fetchData();
      });
  }

  compressionDisable(data) {
    this.tapeLibraryApiService
      .modifyTapeDriveUsingPUT({
        tapeLibrarySn: this.item.serialNo,
        tapeDriveModifyRequest: {
          blockSize: data[0].blockSize,
          compressionStatus: 'DISABLE',
          status: data[0].status
        },
        driverSn: data[0].driverSn,
        memberEsn: this.node?.remoteEsn
      })
      .subscribe(res => {
        this.driveTable.fetchData();
      });
  }

  improt(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('system_tape_load_label'),
        lvComponentParams: {
          node: this.node,
          tapeLibrarySn: this.item.serialNo,
          tapeLabel: data[0].tapeLabel
        },
        lvContent: SelectSpecificDriveComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as SelectSpecificDriveComponent;
          content.valid$.subscribe(res => {
            modal.lvOkDisabled = !res;
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as SelectSpecificDriveComponent;
            content.onOK().subscribe(
              res => {
                resolve(true);
                this.tapeTable.fetchData();
              },
              error => resolve(false)
            );
          });
        }
      }
    });
  }

  export(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('system_tape_unload_label'),
        lvComponentParams: {
          node: this.node,
          tapeLibrarySn: this.item.serialNo,
          tapeLabel: data[0].tapeLabel
        },
        lvContent: SelectSpecificSlotComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as SelectSpecificSlotComponent;
          content.valid$.subscribe(res => {
            modal.lvOkDisabled = !res;
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as SelectSpecificSlotComponent;
            content.onOK().subscribe(
              res => {
                resolve(true);
                this.tapeTable.fetchData();
              },
              error => resolve(false)
            );
          });
        }
      }
    });
  }

  setBlockSize(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      ...{
        lvWidth: MODAL_COMMON.normalWidth,
        lvOkDisabled: true,
        lvHeader: this.i18n.get('system_set_block_size_label'),
        lvComponentParams: {
          node: this.node,
          rowItem: data[0],
          tapeLibrarySn: this.item.serialNo
        },
        lvContent: SetBlockSizeComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as SetBlockSizeComponent;
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as SetBlockSizeComponent;
            content.onOk().subscribe(
              res => {
                resolve(true);
                this.driveTable.fetchData();
              },
              error => resolve(false)
            );
          });
        }
      }
    });
  }

  delete(data) {
    this.tapeLibraryApiService
      .deleteTapeUsingDELETE({
        tapeLibrarySn: data[0].tapeLibrarySn,
        tapeLabel: data[0].tapeLabel,
        memberEsn: this.node?.remoteEsn
      })
      .subscribe(res => {
        this.tapeTable.fetchData();
      });
  }

  indentifyTape(data) {
    this.tapeLibraryApiService
      .identifyTapeUsingPUT({
        tapeLibrarySn: data[0].tapeLibrarySn,
        tapeLabel: data[0].tapeLabel,
        memberEsn: this.node?.remoteEsn
      })
      .subscribe(res => {
        this.tapeTable.fetchData();
      });
  }

  markEmpty(data) {
    this.warningMessageService.create({
      content: this.i18n.get('system_mark_empty_tip_label', [
        data[0].tapeLabel
      ]),
      onOK: () => {
        this.tapeLibraryApiService
          .setTapeEmptyUsingPUT({
            tapeLibrarySn: data[0].tapeLibrarySn,
            tapeLabel: data[0].tapeLabel,
            memberEsn: this.node?.remoteEsn
          })
          .subscribe(res => {
            this.tapeTable.fetchData();
          });
      }
    });
  }

  earse(data) {
    this.warningMessageService.create({
      content: this.i18n.get('system_earse_tip_label', [data[0].tapeLabel]),
      onOK: () => {
        this.tapeLibraryApiService
          .eraseTapeUsingPUT({
            tapeLibrarySn: data[0].tapeLibrarySn,
            tapeLabel: data[0].tapeLabel,
            memberEsn: this.node?.remoteEsn
          })
          .subscribe(res => {
            this.tapeTable.fetchData();
          });
      }
    });
  }
}
