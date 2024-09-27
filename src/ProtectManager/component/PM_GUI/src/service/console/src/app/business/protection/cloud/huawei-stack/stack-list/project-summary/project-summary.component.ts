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
import {
  CookieService,
  DataMap,
  DataMapService,
  extendSlaInfo,
  HCSHostInNormalStatus,
  I18NService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { SlaService } from 'app/shared/services/sla.service';
import { assign, each, includes, isEmpty, size, uniq } from 'lodash';

@Component({
  selector: 'aui-project-summary',
  templateUrl: './project-summary.component.html',
  styleUrls: ['./project-summary.component.less']
})
export class ProjectSummaryComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData = {
    data: [],
    total: 0
  };
  source;
  title = this.i18n.get('common_cloud_server_label');

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private cookieService: CookieService,
    private slaService: SlaService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  ngAfterViewInit() {
    if (this.dataTable) {
      this.dataTable.fetchData();
    }
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
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: includes(
            [
              DataMap.Resource_Type.APSResourceSet.value,
              DataMap.Resource_Type.APSZone.value
            ],
            this.source.subType
          )
            ? this.dataMapService.toArray('fcVMLinkStatus')
            : this.dataMapService
                .toArray('HCS_Host_LinkStatus')
                .filter(item => {
                  return includes(
                    [
                      DataMap.HCS_Host_LinkStatus.normal.value,
                      DataMap.HCS_Host_LinkStatus.offline.value,
                      DataMap.HCS_Host_LinkStatus.suspended.value,
                      DataMap.HCS_Host_LinkStatus.error.value,
                      DataMap.HCS_Host_LinkStatus.softDelete.value
                    ],
                    item.value
                  );
                })
        },
        cellRender: {
          type: 'status',
          config: includes(
            [
              DataMap.Resource_Type.APSResourceSet.value,
              DataMap.Resource_Type.APSZone.value
            ],
            this.source.subType
          )
            ? this.dataMapService.toArray('fcVMLinkStatus')
            : this.dataMapService.toArray('HCS_Host_LinkStatus')
        }
      },
      {
        key: 'path',
        name: this.i18n.get('common_location_label'),
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
        key: 'protection_status',
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
        async: true,
        size: 'small',
        columns: cols,
        compareWith: 'uuid',
        colDisplayControl: false,
        fetchData: (filter: Filters, args: {}) => {
          this.getData(filter, args);
        },
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
    this.title = this.i18n.get('common_cloud_server_label');
  }

  getData(filters: Filters, args: any) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    const defaultConditions =
      this.source.subType === DataMap.Resource_Type.openStackProject.value
        ? {
            subType: [DataMap.Resource_Type.openStackCloudServer.value],
            path: [['=~'], `${this.source.path}/`]
          }
        : includes([DataMap.Resource_Type.APSZone.value], this.source.subType)
        ? {
            subType: DataMap.Resource_Type.APSCloudServer.value,
            path: [['=~'], this.source.path + '/'],
            rootUuid: this.source.rootUuid
          }
        : {
            subType: this.source.sub_Type,
            path: [['=~'], this.source.path],
            type: ResourceType.CLOUD_HOST
          };

    if (!isEmpty(filters.conditions_v2)) {
      const filterConditions = JSON.parse(filters.conditions_v2);
      if (
        filterConditions.status &&
        includes(
          filterConditions.status,
          DataMap.HCS_Host_LinkStatus.error.value
        )
      ) {
        const [, ...status] = filterConditions.status;
        filterConditions.status = [
          ['in'],
          ...uniq([...status, ...HCSHostInNormalStatus])
        ];
      }
      assign(defaultConditions, filterConditions);
    }

    assign(params, { conditions: JSON.stringify(defaultConditions) });

    if (!!size(filters.sort)) {
      assign(params, { orders: filters.orders });
    }

    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      each(res.records, item => {
        assign(item, {
          sub_type: item.subType,
          status: includes(
            [
              DataMap.Resource_Type.openStackProject.value,
              DataMap.Resource_Type.APSZone.value
            ],
            this.source.subType
          )
            ? item.extendInfo?.status
            : JSON.parse(item.extendInfo?.host || '{}')?.status
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
