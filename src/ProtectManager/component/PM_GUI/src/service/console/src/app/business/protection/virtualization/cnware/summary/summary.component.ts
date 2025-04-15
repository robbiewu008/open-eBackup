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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  AppService,
  CAPACITY_UNIT,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  first,
  includes,
  isEmpty,
  isFunction,
  isNumber,
  isString,
  map,
  size
} from 'lodash';

@Component({
  selector: 'aui-cnware-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  item;
  tableConfig: TableConfig;
  tableData: TableData;
  source: any;
  sourceType: string;
  unitconst = CAPACITY_UNIT;
  isVm = false;
  dataMap = DataMap;
  activeIndex;

  tabs = [
    {
      id: 'host',
      subType: DataMap.Resource_Type.cNwareHost.value,
      label: this.i18n.get('common_host_label'),
      hide: false,
      hideFn: () => {
        return !includes(
          [DataMap.Resource_Type.cNwareCluster.value],
          this.sourceType
        );
      }
    },
    {
      id: 'vm',
      subType: DataMap.Resource_Type.cNwareVm.value,
      label: this.i18n.get('common_virtual_machine_label'),
      hide: false
    }
  ];

  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  @ViewChild('headerTpl', { static: true }) headerTpl;

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private appService: AppService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    if (this.item) {
      this.initDetailData(this.item);
    }
    this.getTabs();
    this.initConfig();
    this.getResourceDetail();
    this.getModalHeader();
  }

  getModalHeader() {
    if (!this.isVm) {
      this.modal.setProperty({ lvHeader: this.headerTpl });
    }
  }

  getTabs() {
    each(this.tabs, tab => {
      if (isFunction(tab.hideFn)) {
        tab.hide = tab.hideFn();
      }
    });
    this.tabs = [...this.tabs];
  }

  initConfig() {
    if (!this.isVm) {
      return;
    }
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
        key: 'bus',
        name: this.i18n.get('common_bus_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('cnwareDiskType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('cnwareDiskType')
        }
      },
      {
        key: 'sla',
        name: this.i18n.get('protection_associate_sla_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('slaAssociateStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('slaAssociateStatus')
        }
      },
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        sort: true,
        cellRender: this.sizeTpl
      },
      {
        key: 'storage',
        name: this.i18n.get('common_datastore_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: cols,
        compareWith: 'uuid',
        colDisplayControl: false,
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getResourceDetail() {
    if (!this.isVm) {
      return;
    }
    this.appUtilsService
      .getResourcesDetails(this.source, DataMap.Resource_Type.cNwareDisk.value)
      .subscribe(recordsTemp => {
        const protectedDisk = this.getProtectedDisk();
        const allDisk =
          !isEmpty(this.source?.protectedObject) &&
          this.source.protectedObject?.extParameters?.all_disk === 'True';

        each(recordsTemp, item => {
          assign(item, JSON.parse(item.extendInfo?.details), {
            sla: allDisk ? true : includes(protectedDisk, item.uuid)
          });
        });
        this.tableData = {
          data: recordsTemp,
          total: size(recordsTemp)
        };
      });
  }

  getProtectedDisk() {
    if (
      !isEmpty(this.source?.protectedObject) &&
      !isEmpty(this.source.protectedObject?.extParameters?.disk_info)
    ) {
      return map(
        this.source.protectedObject?.extParameters?.disk_info,
        item => {
          return isString(item) ? JSON.parse(item)?.uuid : item.uuid;
        }
      );
    }
    return [];
  }

  initDetailData(data: any) {
    this.sourceType = data.sub_type || data.subType;
    this.source = data;
    this.isVm = this.sourceType === DataMap.Resource_Type.cNwareVm.value;
  }
}
