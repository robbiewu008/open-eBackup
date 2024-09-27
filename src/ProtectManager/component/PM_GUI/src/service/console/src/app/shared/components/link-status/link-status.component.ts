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
import { DataMap, I18NService } from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { each, size, map, some } from 'lodash';
import { DataMapService } from 'app/shared';
@Component({
  selector: 'aui-link-status',
  templateUrl: './link-status.component.html',
  styleUrls: ['./link-status.component.less']
})
export class LinkStatusComponent implements OnInit {
  tableConfig: TableConfig;
  tableData: TableData;
  data;
  type;
  isOffline: boolean;
  constructor(
    public i18n: I18NService,
    public dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initTable();
    this.initData();
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'cluster_name',
        name: this.i18n.get('protection_cluster_node_label')
      },
      {
        key: 'link_status',
        name: this.i18n.get('common_status_label'),
        cellRender: {
          type: 'status',
          config:
            this.type === 'archive'
              ? this.dataMapService.toArray('Archive_Storage_Status')
              : this.dataMapService.toArray('resource_Host_LinkStatus')
        }
      },
      {
        key: 'end_point',
        name: this.i18n.get('common_ip_address_label')
      }
    ];
    this.tableConfig = {
      table: {
        compareWith: 'cluster_name',
        columns: cols,
        size: 'small',
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showTotal: false,
        winTablePagination: true,
        showPageSizeOptions: false
      }
    };
  }

  initData() {
    let dataInfo = [];
    if (this.type === 'archive') {
      this.isOffline = some(
        this.data,
        item => item.status === DataMap.Archive_Storage_Status.offline.value
      );
      dataInfo = map(this.data, item => {
        return {
          cluster_name: item.clusterName,
          link_status: item.status,
          end_point: item.clusterIp
        };
      });
    } else {
      const tmp = JSON.parse(this.data.extendInfo.connection_result);
      this.isOffline = some(
        tmp,
        item => item.status === DataMap.resource_Host_LinkStatus.offline.value
      );
      dataInfo = map(tmp, item => {
        const status = Number(item.link_status);
        return {
          cluster_name: item.cluster_name,
          link_status: status,
          end_point: item.end_point
        };
      });
    }
    this.tableData = {
      data: dataInfo,
      total: size(dataInfo)
    };
  }
}
