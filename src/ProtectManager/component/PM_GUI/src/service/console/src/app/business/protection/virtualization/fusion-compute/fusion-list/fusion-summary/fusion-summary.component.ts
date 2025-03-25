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
import {
  AppService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, each, isEmpty, size } from 'lodash';
import { forkJoin, Observable, of } from 'rxjs';
import { mergeMap } from 'rxjs/operators';

@Component({
  selector: 'aui-fusion-summary',
  templateUrl: './fusion-summary.component.html',
  styleUrls: ['./fusion-summary.component.less']
})
export class FusionSummaryComponent implements OnInit {
  resourceType = ResourceType;
  tableConfig: TableConfig;
  tableData = {
    data: [],
    total: 0
  };
  detailData;
  source;
  @ViewChild('capacityTpl', { static: true }) capacityTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private appService: AppService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initConfig(); // 只做表格初始化操作
    this.showTableData();
  }

  initConfig() {
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
        key: 'slot',
        name: this.i18n.get('common_slot_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
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
        key: 'capacity',
        name: this.i18n.get('common_capacity_label'),
        sort: true,
        cellRender: this.capacityTpl,
        thAlign: 'right'
      },
      {
        key: 'datastore',
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
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  initDetailData(data: any) {
    this.source = data;
  }

  showTableData() {
    let selectAll = false;
    let selectDisk = [];
    if (!isEmpty(this.source?.protectedObject)) {
      const diskInfo = this.source.protectedObject?.extParameters?.disk_info;
      if (isEmpty(diskInfo)) {
        selectAll = true;
      } else {
        selectDisk = diskInfo.map(item => JSON.parse(item)?.id);
      }
    }

    this.appUtilsService
      .getResourcesDetails(this.source, '', {}, {}, false)
      .subscribe(res => {
        const totalData = [...res];
        each(totalData, (item: any) => {
          assign(item, {
            slot: `${item?.extendInfo.pciType}(${item?.extendInfo.sequenceNum})`,
            datastore: item?.extendInfo?.datastoreName,
            capacity: item?.extendInfo?.quantityGB,
            sla: selectAll ? true : selectDisk.includes(item.uuid)
          });
        });
        this.tableData = {
          data: totalData,
          total: size(totalData)
        };
      });
  }
}
