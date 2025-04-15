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
import { FilterItem } from '@iux/live';
import {
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
import {
  toString,
  assign,
  defer,
  size,
  map,
  isEmpty,
  each,
  find,
  isString,
  replace
} from 'lodash';

@Component({
  selector: 'aui-summary-database-list',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  dbInfo;
  type = DataMap.Resource_Type.oracle.value;
  dataMap = DataMap;

  tableConfig: TableConfig;
  tableData: TableData;
  storageConfig: TableConfig;
  storageData: TableData;
  protocolOpts: FilterItem[] = this.dataMapService
    .toArray('dataProtocolType')
    .filter(v => (v.isLeaf = true));
  @ViewChild('instNameTpl', { static: true }) instNameTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  initDetailData(data) {
    this.source = data;
    this.getInfo(data);
  }

  getInfo(data) {
    this.dbInfo = assign(data, {
      db_type: data.type,
      link_status: toString(data.link_status)
    });
  }

  initConfig() {
    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label')
          },
          {
            key: 'endpoint',
            name: this.i18n.get('common_ip_address_label')
          },
          {
            key: 'linkStatus',
            name: this.i18n.get('common_status_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('resource_LinkStatus_Special')
            }
          },
          {
            key: 'inst_name',
            name: this.i18n.get('commom_owned_instance_label'),
            cellRender: this.instNameTpl
          }
        ],
        compareWith: 'uuid',
        colDisplayControl: false,
        trackByFn: (_, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        winTablePagination: true,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showPageSizeOptions: false
      }
    };
    this.storageConfig = {
      table: {
        async: false,
        columns: [
          {
            key: 'ipList',
            name: this.i18n.get('common_management_ip_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'port',
            name: this.i18n.get('common_port_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'transport_protocol',
            name: this.i18n.get('common_protocol_label'),
            filter: {
              type: 'select',
              options: this.protocolOpts,
              isMultiple: true,
              showCheckAll: true
            }
          }
        ],
        size: 'small',
        colDisplayControl: false,
        showLoading: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: 10
      }
    };
    defer(() => {
      const storages = JSON.parse(this.source.extendInfo?.storages || '[]');
      this.storageData = {
        data: storages,
        total: size(storages)
      };
      if (this.source?.subType === DataMap.Resource_Type.oracleCluster.value) {
        this.protectedResourceApiService
          .ShowResource({
            resourceId: this.source.environment?.uuid || this.source.uuid
          })
          .subscribe((res: any) => {
            this.tableData = {
              data: res.dependencies?.agents,
              total: size(res.dependencies?.agents)
            };
            let instances;
            try {
              instances = JSON.parse(this.source.extendInfo?.instances);
            } catch (error) {
              instances = [];
            }
            if (!isEmpty(instances)) {
              each(this.tableData?.data, item => {
                assign(item, {
                  extendInfo: {
                    ...item.extendInfo,
                    inst_name: find(instances, { hostId: item.uuid })?.inst_name
                  }
                });
              });
            }
          });
      } else {
        this.tableData = {
          data: this.source.dependencies?.agents,
          total: size(this.source.dependencies?.agents)
        };
      }
    });
  }
}
