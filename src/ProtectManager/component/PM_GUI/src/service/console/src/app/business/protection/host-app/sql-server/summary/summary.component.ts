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
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
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
  TableCols
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { SlaService } from 'app/shared/services/sla.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  toString,
  assign,
  size,
  each,
  isNumber,
  split,
  find,
  filter,
  isEmpty
} from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-sql-server-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  dbInfo;
  type = DataMap.Resource_Type.SQLServerInstance.value;
  nodeConfig;
  nodeData;
  groupConfig;
  groupData;
  databaseConfig;
  databaseData;
  dataMap = DataMap;

  @ViewChild('slaComplianceExtraTpl', { static: true })
  slaComplianceExtraTpl: TemplateRef<any>;
  constructor(
    private appUtilsService: AppUtilsService,
    private i18n: I18NService,
    private slaService: SlaService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private cdr: ChangeDetectorRef,
    private virtualScroll: VirtualScrollService
  ) {}

  ngOnInit() {
    this.initTable();
    this.getDatabase();
    this.getProxy();
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

  initTable() {
    const nodeCols: TableCols[] = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'ip',
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
    ];
    const groupCols: TableCols[] = [
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
        key: 'parentName',
        name: this.i18n.get('insight_report_belong_cluster_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'instance',
        name: this.i18n.get('protection_database_instance_label'),
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
    const databaseCols: TableCols[] = [
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
        key: 'clusterOrAgent',
        name: this.i18n.get('protection_host_cluster_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'parentName',
        name: this.i18n.get('protection_database_instance_label'),
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
    this.nodeConfig = {
      table: {
        columns: nodeCols,
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
    this.groupConfig = {
      table: {
        columns: groupCols,
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
    this.databaseConfig = {
      table: {
        columns: databaseCols,
        showLoading: false,
        colDisplayControl: false,
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getDatabase(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE
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
        map(res => {
          each(res.records, item => {
            assign(item, {
              sub_type: item.subType,
              auth_status: DataMap.Verify_Status.true.value,
              clusterOrAgent: item.extendInfo?.hostName
            });
            extendSlaInfo(item);
          });
          return res;
        })
      )
      .pipe(
        map(res => {
          each(res.records, item => {
            assign(item, {});
            extendSlaInfo(item);
          });
          return res;
        })
      )
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
          res.totalCount === 0
        ) {
          this.databaseData = {
            total: res.totalCount,
            data: recordsTemp
          };
          this.cdr.detectChanges();
          return;
        }
        this.getDatabase(recordsTemp, startPage);
      });
  }

  getGroup() {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE * 10
    };

    const defaultConditions = {
      subType: DataMap.Resource_Type.SQLServerGroup.value,
      parentUuid: [['=='], this.source.uuid]
    };

    assign(params, {
      conditions: JSON.stringify(defaultConditions)
    });

    this.protectedResourceApiService
      .ListResources(params)
      .pipe(
        map(res => {
          each(res.records, item => {
            assign(item, {
              sub_type: item.subType,
              auth_status: DataMap.Verify_Status.true.value,
              clusterOrAgent: item.extendInfo?.hostName
            });
            extendSlaInfo(item);
          });
          return res;
        })
      )
      .pipe(
        map(res => {
          each(res.records, item => {
            assign(item, {});
            extendSlaInfo(item);
          });
          return res;
        })
      )
      .subscribe(res => {
        this.groupData = {
          total: res.totalCount,
          data: res.records
        };
        this.cdr.detectChanges();
      });
  }

  getProxy() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.SQLServerInstance.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const dataList = [];
        const endpoints = !!this.source.extendInfo?.agents
          ? JSON.parse(this.source.extendInfo.agents)
          : [{ uuid: this.source.extendInfo?.hostId }];
        each(resource, item => {
          const tmp = item.environment;
          if (
            find(endpoints, val => val.uuid === tmp.uuid) &&
            tmp.extendInfo.scenario === DataMap.proxyHostType.external.value
          ) {
            dataList.push({
              key: tmp.uuid,
              name: tmp?.name,
              ip: tmp?.endpoint,
              linkStatus: tmp?.linkStatus
            });
          }
        });
        this.nodeData = {
          data: dataList,
          total: dataList.length
        };
      }
    );
  }
}
