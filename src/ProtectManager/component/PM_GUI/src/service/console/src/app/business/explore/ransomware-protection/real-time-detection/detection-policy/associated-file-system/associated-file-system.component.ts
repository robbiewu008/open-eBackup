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
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  IODETECTPOLICYService
} from 'app/shared';
import { ProtectedResourcePageListResponse } from 'app/shared/api/models';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { assign, each, filter, isEmpty, map } from 'lodash';

@Component({
  selector: 'aui-associated-file-system',
  templateUrl: './associated-file-system.component.html',
  styleUrls: ['./associated-file-system.component.less']
})
export class AssociatedFileSystemComponent implements OnInit, AfterViewInit {
  @Input() rowData;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('storageDeviceTpl', { static: true })
  storageDeviceTpl: TemplateRef<any>;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private virtualScroll: VirtualScrollService,
    private ioDetectPolicyService: IODETECTPOLICYService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.virtualScroll.getScrollParam(260);
    this.initConfig();
    this.getDevice();
    window.addEventListener('resize', () => {
      this.virtualScroll.getScrollParam(260);
      this.dataTable.setTableScroll(this.virtualScroll.scrollParam);
    });
  }

  getDevice() {
    this.appUtilsService
      .getCyberEngineStorage()
      .subscribe((res: ProtectedResourcePageListResponse) => {
        const devices = filter(
          res.records,
          item =>
            item.subType !==
            DataMap.cyberDeviceStorageType.OceanStorPacific.value
        );
        const deviceFilterMap = map(devices, item => {
          return {
            key: item.uuid,
            label: item.name,
            value: item.name
          };
        });
        each(this.tableConfig.table.columns, item => {
          if (item.key === 'parentName') {
            assign(item, {
              filter: {
                type: 'select',
                isMultiple: true,
                showCheckAll: true,
                options: deviceFilterMap
              }
            });
          }
        });
        this.dataTable.init();
      });
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'fsName',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'isIoDetectEnabled',
        name: this.i18n.get('protection_protected_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('ioDetectEnabled')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('ioDetectEnabled')
        }
      },
      {
        key: 'parentName',
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
    ];

    this.tableConfig = {
      table: {
        compareWith: 'id',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        scroll: this.virtualScroll.scrollParam,
        fetchData: (filters: Filters) => this.getData(filters),
        trackByFn: (_, item) => {
          return item.id;
        }
      },
      pagination: {
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS,
        showPageSizeOptions: true
      }
    };
  }

  getData(filters: Filters) {
    const params = {
      pageNum: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (!isEmpty(conditions.isIoDetectEnabled)) {
        assign(params, {
          ioDetectStatus: conditions.isIoDetectEnabled
        });
        delete conditions.isIoDetectEnabled;
      }
      if (!isEmpty(conditions.parentName)) {
        assign(params, {
          deviceNames: conditions.parentName
        });
        delete conditions.parentName;
      }
      assign(params, conditions);
    }
    this.ioDetectPolicyService
      .getPolicyAssociationFsById({ policyId: this.rowData?.id, ...params })
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }
}
