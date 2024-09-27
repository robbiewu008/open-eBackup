import { Component, OnInit, AfterViewInit, ViewChild } from '@angular/core';
import {
  DataMap,
  I18NService,
  GlobalService,
  DataMapService,
  ResourceType,
  ProtectedResourceApiService
} from 'app/shared';

import {
  TableCols,
  ProTableComponent,
  Filters
} from 'app/shared/components/pro-table';
import { get, isArray, isNil, map as __map, size } from 'lodash';

@Component({
  selector: 'aui-click-house-summary-cluster',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit, AfterViewInit {
  source;
  sourceType;
  dataMap = DataMap;
  tableConfig;
  tableData;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private globalService: GlobalService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initTable();
    this.globalService.getState('registerClickHouseCluster').subscribe(res => {
      this.source = { ...this.source, ...res };
      this.dataTable.fetchData();
    });
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  initDetailData(data) {
    this.source = data;
    this.sourceType = data.resourceType;
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'hostname',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_label'),
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
          options: this.dataMapService.toArray('clickHouse_node_status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('clickHouse_node_status')
        }
      },
      {
        key: 'clientPath',
        name: this.i18n.get('common_client_path_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'ip',
        name: this.i18n.get('common_business_ip_label'),
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
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        fetchData: (filters: Filters) => {
          this.getNodeList(filters);
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getNodeList(filters: Filters) {
    const params = {
      resourceId: this.source.uuid
    };
    this.protectedResourceApiService.ShowResource(params).subscribe(res => {
      const nodes = this.getShowTableData(
        get(res, ['dependencies', 'children'])
      ); // 总数据
      if (filters.conditions) {
        const conditions = JSON.parse(filters.conditions);
        const _nodes = nodes.filter(item => {
          for (const key in conditions) {
            if (Object.prototype.hasOwnProperty.call(conditions, key)) {
              if (key === 'status') {
                if (isArray(conditions[key])) {
                  if (!conditions[key].includes(item[key])) {
                    return false;
                  }
                }
              } else {
                if (!item[key].includes(conditions[key])) {
                  return false;
                }
              }
            }
          }
          return true;
        });
        this.tableData = {
          data: _nodes,
          total: size(_nodes)
        };
      } else {
        this.tableData = {
          data: nodes,
          total: size(nodes)
        };
      }
    });
  }

  getShowTableData(data) {
    if (isNil(data)) return [];

    return data?.map(item => {
      return {
        hostname: item?.dependencies?.agents[0]?.name,
        endpoint: item?.dependencies?.agents[0]?.endpoint,
        status: item.extendInfo.status,
        clientPath: item.extendInfo.clientPath,
        ip: item.extendInfo.ip,
        port: item.extendInfo.port
      };
    });
  }
}
