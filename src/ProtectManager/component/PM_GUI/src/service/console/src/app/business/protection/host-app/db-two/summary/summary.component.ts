import { Component, OnInit, ViewChild } from '@angular/core';
import {
  I18NService,
  DataMapService,
  ProtectedResourceApiService,
  DataMap
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig
} from 'app/shared/components/pro-table';
import { get, includes, map, map as _map, toString as _toString } from 'lodash';

@Component({
  selector: 'aui-db-two-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  data = {} as any;
  formItems = [];
  tableConfig: TableConfig;
  tableData;
  resSubType;
  dataMap = DataMap;
  isInstance = false;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initFormItems();
    this.initConfig();
    this.getDetails();
  }

  initDetailData(data: any) {
    this.data = data;
    this.isInstance = includes(
      [
        DataMap.Resource_Type.dbTwoInstance.value,
        DataMap.Resource_Type.dbTwoClusterInstance.value
      ],
      this.data.subType
    );
  }

  initFormItems() {
    this.formItems =
      this.data.subType === DataMap.Resource_Type.dbTwoCluster.value
        ? [
            [
              {
                key: 'name',
                value: this.data.name,
                label: this.i18n.get('common_name_label')
              }
            ],
            [
              {
                key: 'type',
                value: this.dataMapService.getLabel(
                  'dbTwoType',
                  this.data.clusterType
                ),
                label: this.i18n.get('common_type_label')
              }
            ]
          ]
        : [
            [
              {
                key: 'name',
                value: this.data.name,
                label: this.i18n.get('common_name_label')
              }
            ],
            [
              {
                key: 'parentName',
                value: this.data.clusterOrHostName,
                label: this.i18n.get('protection_host_cluster_name_label')
              }
            ],
            [
              {
                key: 'version',
                value: this.data.version,
                label: this.i18n.get('common_version_label')
              }
            ]
          ];
  }

  initConfig() {
    const cols: TableCols[] = [
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
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        async: false
      },
      pagination: null
    };
  }

  getDetails() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.data.uuid })
      .subscribe(res => {
        const dataList =
          get(res, 'dependencies.agents') ||
          get(res, 'dependencies.children') ||
          [];
        this.tableData = {
          data: map(dataList, item => {
            return {
              name: item.name,
              endpoint: item.endpoint || item.path,
              linkStatus: item.linkStatus || item.extendInfo?.linkStatus
            };
          }),
          total: dataList.length
        };
      });
  }
}
