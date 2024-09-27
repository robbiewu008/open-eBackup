import { Component, OnInit, ChangeDetectorRef } from '@angular/core';
import {
  I18NService,
  DataMapService,
  ProtectedResourceApiService,
  DataMap
} from 'app/shared';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';
import { get, map, size, toString as _toString } from 'lodash';

@Component({
  selector: 'aui-summary-instance',
  templateUrl: './summary-instance.component.html',
  styleUrls: ['./summary-instance.component.less']
})
export class SummaryInstanceComponent implements OnInit {
  tableData;
  resSubType;
  formItems = [];
  data = {} as any;
  dataMap = DataMap;
  tableConfig: TableConfig;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
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
  }

  initFormItems() {
    this.formItems = [
      [
        {
          key: 'name',
          value: this.data.name,
          label: this.i18n.get('common_name_label')
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
        compareWith: 'ip',
        columns: cols,
        virtualScroll: true,
        scrollFixed: true,
        scroll: { y: '420px' },
        colDisplayControl: false
      },
      pagination: null
    };
  }

  getDetails() {
    if (!this.data) {
      return;
    }
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.data.uuid })
      .subscribe((res: any) => {
        const nodes = map(get(res, 'dependencies.agents'), item => {
          return {
            uuid: item.uuid,
            name: item.name,
            linkStatus: item.linkStatus,
            endpoint: item.endpoint
          };
        });

        this.tableData = {
          data: nodes,
          total: size(nodes)
        };
        this.cdr.detectChanges();
      });
  }
}
