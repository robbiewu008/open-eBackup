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
import { Component, OnInit, ViewChild } from '@angular/core';
import {
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, get, map, size } from 'lodash';

@Component({
  selector: 'aui-summary-database',
  templateUrl: './summary-database.component.html',
  styleUrls: ['./summary-database.component.less']
})
export class SummaryDatabaseComponent implements OnInit {
  source;
  resourceType = DataMap.Resource_Type;
  subType = DataMap.Resource_Type.saphanaDatabase.value;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.getDetails();
  }

  initDetailData(data) {
    this.source = assign(data, {
      subType: data.sub_type || data.subType
    });
    this.subType = data.sub_type || data.subType;
  }

  initConfig() {
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
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'linkStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('resource_LinkStatus_Special')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('resource_LinkStatus_Special')
        }
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_address_label'),
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
      pagination: null
    };
  }

  getDetails() {
    if (!this.source) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({
        resourceId:
          get(this.source, 'extendInfo.sapHanaDbType') ===
          DataMap.saphanaDatabaseType.systemdb.value
            ? this.source.parentUuid
            : this.source.uuid
      })
      .subscribe((res: any) => {
        const nodes = map(get(res, 'dependencies.agents'), item => {
          return {
            uuid: item.uuid,
            name: item.name,
            linkStatus: item.linkStatus,
            endpoint: item.endpoint
          };
        });

        this.tableData = {
          data: nodes,
          total: size(nodes)
        };
      });
  }
}
