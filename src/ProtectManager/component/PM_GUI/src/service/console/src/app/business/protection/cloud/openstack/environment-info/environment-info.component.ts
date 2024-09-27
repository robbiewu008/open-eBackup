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
import { ModalRef } from '@iux/live';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, isEmpty, size } from 'lodash';

@Component({
  selector: 'aui-environment-info',
  templateUrl: './environment-info.component.html',
  styleUrls: ['./environment-info.component.less']
})
export class EnvironmentInfoComponent implements OnInit {
  rowItem;
  resourceType = ResourceType;
  dataMap = DataMap;
  nodeTableConfig: TableConfig;
  nodeTableData: TableData;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.updateOpts();
    this.initConfig();
    this.getAgents();
  }

  updateOpts() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  initConfig() {
    this.nodeTableConfig = {
      table: {
        async: false,
        colDisplayControl: false,
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
          }
        ]
      },
      pagination: {
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false
      }
    };

    this.tableConfig = {
      table: {
        colDisplayControl: false,
        columns: [
          {
            key: 'name',
            name: this.i18n.get('common_name_label'),
            filter: {
              type: 'search'
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
        ],
        fetchData: (filters: Filters) => this.getProject(filters)
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS
      }
    };
  }

  getAgents() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowItem.uuid })
      .subscribe((res: any) => {
        this.nodeTableData = {
          data: res.dependencies?.agents,
          total: size(res.dependencies?.agents)
        };
      });
  }

  getProject(filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const defaultConditions = {
      parentUuid: this.rowItem.uuid
    };

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      assign(defaultConditions, conditionsTemp);
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
    });
  }
}
