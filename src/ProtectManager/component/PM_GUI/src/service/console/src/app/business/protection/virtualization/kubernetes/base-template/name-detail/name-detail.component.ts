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
import { AfterViewInit, Component, OnInit, ViewChild } from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  extendSlaInfo,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { SlaService } from 'app/shared/services/sla.service';
import { assign, each, isEmpty, isUndefined, size } from 'lodash';

@Component({
  selector: 'aui-name-detail',
  templateUrl: './name-detail.component.html',
  styleUrls: ['./name-detail.component.less']
})
export class NameDetailComponent implements OnInit, AfterViewInit {
  source;
  tableConfig: TableConfig;
  tableData: TableData;
  sourceType: string;
  title = 'StatefulSet';

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit(): void {
    this.initConfig();
  }
  initConfig(): void {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        sort: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'parentName',
        name: this.i18n.get('protection_belong_namespace_label'),
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
            overflow: true,
            click: data => {
              this.slaService.getDetail({
                uuid: data.sla_id,
                name: data.sla_name
              });
            }
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
    ];
    this.tableConfig = {
      table: {
        columns: cols,
        async: true,
        colDisplayControl: false,
        fetchData: (filter: Filters, args: {}) => {
          this.getStatefulSet(filter, args);
        }
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
  initDetailData(data: any) {
    this.sourceType = data.sub_type;
    this.source = data;
    if (
      this.sourceType === DataMap.Resource_Type.kubernetesNamespaceCommon.value
    ) {
      this.title = this.i18n.get('protection_kubernetes_dataset_label');
    }
  }

  getStatefulSet(filters: Filters, args: any) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    const defaultConditions = {
      subType: [DataMap.Resource_Type.KubernetesStatefulset.value],
      path: [['=~'], this.source.path]
    };

    if (
      this.sourceType === DataMap.Resource_Type.kubernetesNamespaceCommon.value
    ) {
      delete defaultConditions.path;
      assign(defaultConditions, {
        subType: [DataMap.Resource_Type.kubernetesDatasetCommon.value],
        parentUuid: this.source.uuid
      });
    }

    if (!isEmpty(filters.conditions_v2)) {
      const conditionsTemp = JSON.parse(filters.conditions_v2);
      if (conditionsTemp.equipment) {
        assign(conditionsTemp, {
          environment: {
            name: conditionsTemp.equipment
          }
        });
        delete conditionsTemp.equipment;
      }
      if (conditionsTemp.equipmentType) {
        if (isEmpty(conditionsTemp.environment)) {
          assign(conditionsTemp, {
            environment: {
              subType: conditionsTemp.equipmentType
            }
          });
        } else {
          assign(conditionsTemp.environment, {
            subType: conditionsTemp.equipmentType
          });
        }
        delete conditionsTemp.equipmentType;
      }
      assign(defaultConditions, conditionsTemp);
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        assign(item, {
          sub_type: item.subType,
          cluster: item.environment?.name,
          endpoint: item.environment?.endpoint,
          parentName: item.parentName
        });
        extendSlaInfo(item);
      });
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
    });
  }
}
