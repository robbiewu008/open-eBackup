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
  Component,
  EventEmitter,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService
} from 'app/shared';
import { ProtectedResourcePageListResponse } from 'app/shared/api/models';
import { FsFileExtensionFilterManagementService } from 'app/shared/api/services/fs-file-extension-filter-management.service';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  cloneDeep,
  each,
  filter,
  find,
  first,
  includes,
  isEmpty,
  isUndefined,
  map as _map,
  reject,
  values
} from 'lodash';
import { map } from 'rxjs/operators';
import { FileExtensionDetailComponent } from './file-extension-detail/file-extension-detail.component';

@Component({
  selector: 'aui-interception-file-system',
  templateUrl: './file-system.component.html',
  styleUrls: ['./file-system.component.less']
})
export class FileSystemComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];
  optItems = [];

  @Output() refreshFileSystem = new EventEmitter();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('nameTpl', { static: true })
  nameTpl: TemplateRef<any>;
  @ViewChild('storageDeviceTpl', { static: true })
  storageDeviceTpl: TemplateRef<any>;
  @ViewChild('numTpl', { static: true })
  numTpl: TemplateRef<any>;
  @ViewChild('fsIdTpl', { static: true })
  fsIdTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private virtualScroll: VirtualScrollService,
    private infoMessageService: InfoMessageService,
    private warningMessageService: WarningMessageService,
    private fsFileExtensionFilterManagementService: FsFileExtensionFilterManagementService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
    this.getStorage();
  }

  getStorage() {
    this.appUtilsService
      .getCyberEngineStorage()
      .subscribe((res: ProtectedResourcePageListResponse) => {
        res.records = reject(
          res.records,
          item =>
            item.subType ===
            DataMap.cyberDeviceStorageType.OceanStorPacific.value
        );
        const deviceFilterMap = _map(res.records, item => {
          return {
            key: item.uuid,
            label: item.name,
            value: item.uuid
          };
        });
        each(this.tableConfig.table.columns, item => {
          if (item.key === 'deviceName') {
            assign(item, {
              filter: {
                type: 'select',
                isMultiple: false,
                options: deviceFilterMap
              }
            });
          }
        });
        this.dataTable.init();
      });
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      scan: {
        id: 'scan',
        type: 'primary',
        permission: OperateItems.DeleteResource,
        label: this.i18n.get('common_scan_label'),
        onClick: () => {
          this.fsFileExtensionFilterManagementService
            .scanFsFileBlockUsingPUT({})
            .subscribe(() => this.dataTable.fetchData());
        }
      },
      modifyFilter: {
        id: 'modifyFilter',
        permission: OperateItems.DeleteResource,
        label: this.i18n.get('protection_modify_file_interception_rule_label'),
        disableCheck: data => {
          return isEmpty(data);
        },
        onClick: ([data]) => this.updateFileExtension(data)
      },
      enableBlock: {
        id: 'enableBlock',
        permission: OperateItems.DeleteResource,
        label: this.i18n.get('explore_enable_blocking_files_label'),
        disableCheck: data => {
          return (
            isEmpty(data) ||
            !isEmpty(
              find(
                data,
                item =>
                  item.configStatus ===
                  DataMap.File_Extension_Status.enable.value
              )
            )
          );
        },
        onClick: data => this.enableFileExtension(data)
      },
      disableBlock: {
        id: 'disableBlock',
        permission: OperateItems.DeleteResource,
        label: this.i18n.get('explore_disable_blocking_files_label'),
        disableCheck: data => {
          return (
            isEmpty(data) ||
            !isEmpty(
              find(
                data,
                item =>
                  item.configStatus ===
                  DataMap.File_Extension_Status.disable.value
              )
            )
          );
        },
        onClick: data => this.disableFileExtension(data)
      }
    };

    this.optItems = filter(
      cloneDeep(getPermissionMenuItem(values(opts))),
      item => {
        if (item.id === 'enableBlock') {
          item.type = 'default';
        }
        return !includes(['scan'], item.id);
      }
    );

    const cols: TableCols[] = [
      {
        key: 'fsId',
        name: this.i18n.get('protection_resource_id_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        hidden: true,
        cellRender: this.fsIdTpl
      },
      {
        key: 'fsName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.nameTpl
      },
      {
        key: 'deviceName',
        name: this.i18n.get('protection_storage_device_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: []
        },
        cellRender: this.storageDeviceTpl
      },
      {
        key: 'vstoreName',
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'fileExtensionNumber',
        name: this.i18n.get('explore_blocking_files_rule_num_label'),
        cellRender: this.numTpl
      },
      {
        key: 'configStatus',
        name: this.i18n.get('explore_file_block_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('File_Extension_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('File_Extension_Status')
        }
      },
      {
        key: 'operation',
        width: 130,
        hidden: 'ignoring',
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: this.optItems
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        autoPolling: CommonConsts.TIME_INTERVAL,
        compareWith: 'uuid',
        columns: cols,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        colDisplayControl: {
          ignoringColsType: 'hide'
        },
        fetchData: (filter: Filters, args) => {
          this.getData(filter, args);
        },
        selectionChange: selection => {
          this.selectionData = selection;
        },
        trackByFn: (_, item) => {
          return item.uuid;
        }
      }
    };

    this.optsConfig = getPermissionMenuItem([
      opts.scan,
      opts.enableBlock,
      opts.disableBlock
    ]);
  }

  getData(filters: Filters, args) {
    const params = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      akLoading:
        !isUndefined(args) && args.isAutoPolling ? !args.isAutoPolling : true
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (!isEmpty(conditions.deviceName)) {
        assign(params, {
          deviceId: first(conditions.deviceName)
        });
        delete conditions.deviceName;
      }
      assign(params, conditions);
    }

    this.fsFileExtensionFilterManagementService
      .getFsFileBlockConfigUsingGET(params)
      .pipe(
        map(res => {
          each(res.records, (item: any) => {
            assign(item, {
              uuid: `${item.fileSystemInfo?.deviceId}-${item.fileSystemInfo?.fsId}`
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          total: res.totalCount,
          data: res.records
        };
        this.refreshFileSystem.emit();
      });
  }

  updateFileExtension(rowData) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get(
          'protection_modify_file_interception_rule_label'
        ),
        lvContent: FileExtensionDetailComponent,
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvComponentParams: { rowData, updateOperation: true },
        lvOkDisabled: false,
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as FileExtensionDetailComponent;
            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.dataTable.fetchData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }

  getFileExtensionDetail(rowData) {
    if (!rowData.fileExtensionNumber || rowData.fileExtensionNumber === '0') {
      return;
    }
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvHeader: this.i18n.get('explore_associate_blocking_files_rule_label'),
        lvContent: FileExtensionDetailComponent,
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvComponentParams: { rowData },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }

  enableFileExtension(data) {
    this.infoMessageService.create({
      header: this.i18n.get('explore_enable_file_extension_label'),
      content: this.i18n.get('explore_enable_file_extension_tip_label'),
      width: 450,
      onOK: () => {
        this.fsFileExtensionFilterManagementService
          .modifyFsFileBlockConfigUsingPUT({
            switchFileBlockConfigRequest: {
              fileSystemInfos: _map(data, item => {
                return item.fileSystemInfo;
              }),
              configStatus: DataMap.File_Extension_Status.enable.value
            }
          })
          .subscribe(() => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          });
      }
    });
  }

  disableFileExtension(data) {
    this.warningMessageService.create({
      header: this.i18n.get('explore_disable_file_extension_label'),
      content: this.i18n.get('explore_disable_file_extension_warn_label'),
      onOK: () => {
        this.fsFileExtensionFilterManagementService
          .modifyFsFileBlockConfigUsingPUT({
            switchFileBlockConfigRequest: {
              fileSystemInfos: _map(data, item => {
                return item.fileSystemInfo;
              }),
              configStatus: DataMap.File_Extension_Status.disable.value
            }
          })
          .subscribe(() => {
            this.selectionData = [];
            this.dataTable.setSelections([]);
            this.dataTable.fetchData();
          });
      }
    });
  }
}
