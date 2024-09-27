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
  ClientManagerApiService,
  CommonConsts,
  DataMap,
  DataMapService,
  extendSlaInfo,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { each, filter, includes, map, remove, size } from 'lodash';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-details',
  templateUrl: './details.component.html',
  styleUrls: ['./details.component.less']
})
export class ExchangeDetailsComponent implements OnInit {
  source;
  dbInfo;
  type = DataMap.Resource_Type.oracle.value;
  dataMap = DataMap;
  activeIndex = 0;
  nodeConfig: TableConfig;
  nodeData: TableData;
  databaseConfig: TableConfig;
  databaseData: TableData;
  @ViewChild('nodeTable', { static: false }) nodeTable: ProTableComponent;
  @ViewChild('databaseTable', { static: false })
  databaseTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private appUtilsService: AppUtilsService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.getNodeData();
    this.getDatabaseData();
  }

  initDetailData(data) {
    this.source = data;
  }

  initConfig(): void {
    this.nodeConfig = {
      table: {
        async: false,
        size: 'small',
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'endpoint',
            name: this.i18n.get('common_ip_address_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'linkStatus',
            name: this.i18n.get('common_status_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('resource_LinkStatus_Special')
            },
            filter: {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              options: this.dataMapService.toArray(
                'resource_LinkStatus_Special'
              )
            }
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
    const databaseCols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'sla_name',
        name: this.i18n.get('common_sla_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text',
            overflow: true
          }
        }
      },
      {
        key: 'sla_compliance',
        name: this.i18n.get('common_sla_compliance_label'),
        thExtra: this.slaComplianceExtraTpl,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Sla_Compliance')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Sla_Compliance')
        }
      },
      {
        key: 'protectionStatus',
        name: this.i18n.get('protection_protected_status_label'),
        width: 130,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Protection_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Protection_Status')
        }
      }
    ];

    this.databaseConfig = {
      table: {
        async: false,
        compareWith: 'uuid',
        columns: databaseCols,
        size: 'small',
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: 10
      }
    };
  }

  getDatabaseData() {
    const extParams = {
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.ExchangeDataBase.value]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      res => {
        const resource = filter(res, item => {
          extendSlaInfo(item);
          return item.parentUuid === this.source.uuid;
        });
        this.databaseData = {
          data: resource,
          total: size(resource)
        };
      }
    );
  }

  getNodeData(recordsTemp?, startPage?) {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Host',
        subType: [DataMap.Resource_Type.UBackupAgent.value],
        scenario: [['!='], DataMap.proxyHostType.builtin.value],
        isCluster: [['=='], false]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const agents = this.source.extendInfo.agentUuid.split(';');
        const nodes = remove(resource, item => includes(agents, item['uuid']));
        this.nodeData = {
          data: nodes,
          total: size(nodes)
        };
      }
    );
  }
}
