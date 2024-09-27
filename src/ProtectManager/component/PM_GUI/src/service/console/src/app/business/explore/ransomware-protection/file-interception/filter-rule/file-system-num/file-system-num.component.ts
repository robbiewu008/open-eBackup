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
  FsFileExtensionFilterManagementService,
  I18NService
} from 'app/shared';
import { ProtectedResourcePageListResponse } from 'app/shared/api/models';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, each, find, isEmpty, map as _map, reject } from 'lodash';

@Component({
  selector: 'aui-file-system-num',
  templateUrl: './file-system-num.component.html',
  styleUrls: ['./file-system-num.component.less']
})
export class FileSystemNumComponent implements OnInit, AfterViewInit {
  rowData;
  tableConfig: TableConfig;
  tableData: TableData;
  deviceFilterMap;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('storageDeviceTpl', { static: true })
  storageDeviceTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService,
    private fsFileExtensionFilterManagementService: FsFileExtensionFilterManagementService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initTableConfig();
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
        compareWith: 'fsId',
        columns: [
          {
            key: 'fsName',
            name: this.i18n.get('common_name_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'deviceName',
            name: this.i18n.get('protection_storage_device_label'),
            filter: {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              options: []
            }
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
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        trackByFn: (_, item) => {
          return item.fsId;
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  getData(filters: Filters) {
    const params: any = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      fileExtensionNames: [this.rowData.fileExtensionName]
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
        this.tableData = {
          data: res.records?.fileSystemInfos || [],
          total: res.totalCount || 0
        };
      });
  }
}
