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
import { size } from 'lodash';

@Component({
  selector: 'aui-environment-info-apsara-stack',
  templateUrl: './environment-info-apsara-stack.component.html',
  styleUrls: ['./environment-info-apsara-stack.component.less']
})
export class EnvironmentInfoApsaraStackComponent implements OnInit {
  rowItem;
  tableConfig: TableConfig;
  tableData: TableData;
  dataMap = DataMap;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.getData();
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
        async: false,
        showLoading: false,
        columns: cols,
        size: 'small',
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getData() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowItem?.uuid })
      .subscribe((res: any) => {
        this.tableData = {
          data: res.dependencies?.agents,
          total: size(res.dependencies?.agents)
        };
      });
  }
}
