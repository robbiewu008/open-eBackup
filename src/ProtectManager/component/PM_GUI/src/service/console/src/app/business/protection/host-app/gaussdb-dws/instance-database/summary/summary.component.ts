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
import { ChangeDetectorRef, Component, OnInit } from '@angular/core';
import {
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import { TableCols } from 'app/shared/components/pro-table';
import {
  toString,
  assign,
  toNumber,
  includes,
  each,
  map,
  join,
  set as _set,
  cloneDeep
} from 'lodash';
import { map as _map } from 'rxjs/operators';

@Component({
  selector: 'aui-gaussdb-dws-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  dbInfo;
  type;
  dataMap = DataMap;
  tableConfig;
  tableData;

  constructor(
    private i18n: I18NService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private cdr: ChangeDetectorRef
  ) {}

  ngOnInit() {
    if (this.type === DataMap.Resource_Type.SQLServerGroup.value) {
      this.getDatabase();
    }
  }

  initDetailData(data) {
    this.type = data.subType || data.resourceType;

    if (includes([DataMap.Resource_Type.DWS_Schema.value], this.type)) {
      const sourceData = cloneDeep(data);
      const tableList = sourceData.extendInfo.table.split(',');
      _set(
        sourceData,
        'extendInfo.table',
        join(
          map(tableList, val => `/${val}`),
          ','
        )
      );
      this.source = assign(sourceData, {
        link_status: toNumber(sourceData.linkStatus)
      });
    } else if (
      includes(
        [
          DataMap.Resource_Type.SQLServerDatabase.value,
          DataMap.Resource_Type.DWS_Database.value,
          DataMap.Resource_Type.DWS_Cluster.value
        ],
        this.type
      )
    ) {
      this.source = data;
    } else if (this.type === DataMap.Resource_Type.DWS_Table.value) {
      this.tableData = {
        data: map(data.extendInfo.table.split(','), item => {
          return {
            table: item
          };
        }),
        total: data.extendInfo.table.split(',').length
      };
      this.initDWSTableConfig();
      this.source = data;
    } else if (this.type === DataMap.Resource_Type.SQLServerGroup.value) {
      this.initSQLServerGroupConfig();
      this.source = data;
    }
  }

  initDWSTableConfig() {
    const cols: TableCols[] = [
      {
        key: 'table',
        name: this.i18n.get('protection_table_path_label'),
        sort: true
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  initSQLServerGroupConfig() {
    const cols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        sort: true
      },
      {
        key: 'cluster',
        name: this.i18n.get('insight_report_belong_cluster_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'instance',
        name: this.i18n.get('commom_owned_instance_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getDatabase() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10
    };

    const defaultConditions = {
      subType: DataMap.Resource_Type.SQLServerDatabase.value,
      parentUuid: [['=='], this.source.uuid]
    };

    assign(params, {
      conditions: JSON.stringify(defaultConditions)
    });

    this.protectedResourceApiService
      .ListResources(params)

      .pipe(
        _map(res => {
          each(res.records, item => {
            assign(item, {
              cluster: item.environment?.name,
              instance: item.extendInfo?.instanceName
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
        this.cdr.detectChanges();
      });
  }
}
