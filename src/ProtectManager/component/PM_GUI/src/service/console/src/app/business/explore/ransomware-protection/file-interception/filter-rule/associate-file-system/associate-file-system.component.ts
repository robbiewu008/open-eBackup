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
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  I18NService,
  WarningMessageService
} from 'app/shared';
import { ProtectedResourcePageListResponse } from 'app/shared/api/models';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { Observable, Observer } from 'rxjs';
import {
  assign,
  each,
  find,
  isEmpty,
  map as _map,
  pick,
  reject,
  size
} from 'lodash';
import { FsFileExtensionFilterManagementService } from 'app/shared/api/services/fs-file-extension-filter-management.service';
import { map } from 'rxjs/operators';
import { ModalRef } from '@iux/live';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';

@Component({
  selector: 'aui-associate-file-system',
  templateUrl: './associate-file-system.component.html',
  styleUrls: ['./associate-file-system.component.less']
})
export class AssociateFileSystemComponent implements OnInit, AfterViewInit {
  datas;
  isAssociate;
  tableConfig: TableConfig;
  tableData: TableData;
  selectionData;
  deviceFilterMap;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('nameTpl', { static: true })
  nameTpl: TemplateRef<any>;
  @ViewChild('storageDeviceTpl', { static: true })
  storageDeviceTpl: TemplateRef<any>;

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private virtualScroll: VirtualScrollService,
    private warningMessageService: WarningMessageService,
    private fsFileExtensionFilterManagementService: FsFileExtensionFilterManagementService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.virtualScroll.getScrollParam(300);
    this.initTableConfig();
    this.getStorage();
    window.addEventListener('resize', () => {
      this.virtualScroll.getScrollParam(300);
      this.dataTable.setTableScroll(this.virtualScroll.scrollParam);
    });
  }

  getOkDisabled() {
    this.modal.getInstance().lvOkDisabled = !size(this.selectionData);
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
        this.deviceFilterMap = _map(res.records, item => {
          return {
            key: item.uuid,
            label: item.name,
            value: item.name
          };
        });
        each(this.tableConfig.table.columns, item => {
          if (item.key === 'deviceName') {
            assign(item, {
              filter: {
                type: 'select',
                isMultiple: false,
                options: this.deviceFilterMap
              }
            });
          }
        });
        this.dataTable.init();
      });
  }

  initTableConfig() {
    this.tableConfig = {
      table: {
        compareWith: 'uuid',
        columns: [
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
          }
        ],
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        colDisplayControl: false,
        selectionChange: selection => {
          this.selectionData = selection;
          this.getOkDisabled();
        },
        fetchData: (filter: Filters) => {
          if (this.isAssociate) {
            this.getData(filter);
          } else {
            this.getAssociatedFileSystem(filter);
          }
        },
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        showPageSizeOptions: true,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  getData(filters: Filters) {
    const params = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (!isEmpty(conditions.deviceName)) {
        assign(params, {
          deviceId: find(this.deviceFilterMap, {
            value: conditions.deviceName[0]
          })?.key
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
              uuid: `${item.fileSystemInfo?.deviceId}-${item.fileSystemInfo?.fsId}`,
              ...item.fileSystemInfo
            });
            if (
              this.isAssociate &&
              size(this.datas) === 1 &&
              find(this.selectionData, { uuid: item.uuid })
            ) {
              assign(item, { disabled: true });
            }
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = {
          total: res.totalCount,
          data: res.records
        };
      });
  }

  getAssociatedFileSystem(filters: Filters) {
    const params: any = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      fileExtensionNames: _map(this.datas, 'fileExtensionName')
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (!isEmpty(conditions.deviceName)) {
        assign(params, {
          deviceId: find(this.deviceFilterMap, {
            value: conditions.deviceName[0]
          })?.key
        });
        delete conditions.deviceName;
      }
      assign(params, conditions);
    }

    this.fsFileExtensionFilterManagementService
      .getFileSystemInfoUsingGET(params)
      .subscribe((res: any) => {
        each(res.records?.fileSystemInfos, item => {
          assign(item, {
            uuid: `${item.deviceId}-${item.fsId}`
          });
        });
        this.tableData = {
          data: res.records?.fileSystemInfos || [],
          total: res.totalCount || 0
        };
      });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.isAssociate) {
        this.fsFileExtensionFilterManagementService
          .addFsExtensionFilterUsingPOST({
            importFsSuffixRequest: {
              fileSystemInfos: <any>_map(
                reject(this.selectionData, item => item.disabled),
                item => {
                  return pick(item, [
                    'deviceId',
                    'deviceName',
                    'fsId',
                    'fsName'
                  ]);
                }
              ),
              extensions: _map(this.datas, 'fileExtensionName')
            }
          })
          .subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            error => {
              observer.error(error);
              observer.complete();
            }
          );
      } else {
        this.warningMessageService.create({
          header: this.i18n.get(
            'explore_disassociate_file_system_header_label'
          ),
          content: this.i18n.get('explore_disassociate_file_system_warn_label'),
          onOK: () => {
            this.fsFileExtensionFilterManagementService
              .deleteFsExtensionFilterUsingDELETE({
                deleteFsSuffixRequest: {
                  fileSystemInfos: <any>_map(this.selectionData, item => {
                    return pick(item, [
                      'deviceId',
                      'deviceName',
                      'fsId',
                      'fsName'
                    ]);
                  }),
                  extensions: _map(this.datas, 'fileExtensionName')
                }
              })
              .subscribe(
                () => {
                  observer.next();
                  observer.complete();
                },
                error => {
                  observer.error(error);
                  observer.complete();
                }
              );
          },
          onCancel: () => {
            observer.error(null);
            observer.complete();
          },
          lvAfterClose: result => {
            if (result && result.trigger === 'close') {
              observer.error(null);
              observer.complete();
            }
          }
        });
      }
    });
  }
}
