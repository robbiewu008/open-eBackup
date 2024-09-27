import { Component, OnInit } from '@angular/core';
import {
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
import { assign, find, get, map, size } from 'lodash';

@Component({
  selector: 'aui-tdsql-summary-instance',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  resourceType = DataMap.Resource_Type;
  subType = DataMap.Resource_Type.tdsqlDistributedInstance.value;
  tableConfig: TableConfig;
  tableData: TableData;
  instanceConfig: TableConfig;
  instanceData: TableData;
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
      subType: data.sub_type || data?.subType
    });
    this.subType = data.sub_type || data.subType;
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'ip',
        name: this.i18n.get('common_host_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'name',
        name: this.i18n.get('common_dataplane_ip_label'),
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
      }
    ];
    const instanceCols: TableCols[] = [
      {
        key: 'setId',
        name: this.i18n.get('SETID')
      }
    ];
    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: 10
      }
    };
    this.instanceConfig = {
      table: {
        columns: instanceCols,
        showLoading: false,
        colDisplayControl: false,
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true,
        pageSize: 10
      }
    };
  }

  getDetails() {
    if (!this.source) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.source.uuid })
      .subscribe(res => {
        const clusterGroupInfo = JSON.parse(
          get(res, 'extendInfo.clusterGroupInfo')
        );
        const group = get(clusterGroupInfo, 'group', '{}');
        const agents = get(res, 'dependencies.agents');
        const instances = map(group.setIds, id => {
          return { setId: id };
        });
        const dataNodes = map(group.dataNodes, node => {
          const agent: any = find(agents, { uuid: node.parentUuid });
          return {
            ...node,
            name: `${agent?.name}(${agent?.endpoint})`,
            linkStatus: agent?.linkStatus
          };
        });
        this.tableData = {
          data: dataNodes,
          total: size(dataNodes)
        };
        this.instanceData = {
          data: instances,
          total: size(instances)
        };
      });
  }
}
