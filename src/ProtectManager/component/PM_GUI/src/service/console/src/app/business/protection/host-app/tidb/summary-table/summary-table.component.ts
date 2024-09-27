import { Component, OnInit, ViewChild } from '@angular/core';
import {
  DataMap,
  GlobalService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, map, toNumber } from 'lodash';

@Component({
  selector: 'aui-summary-table',
  templateUrl: './summary-table.component.html',
  styleUrls: ['./summary-table.component.less']
})
export class SummaryTableComponent implements OnInit {
  source;
  dbInfo;
  type;
  dataMap = DataMap;
  tableConfig: TableConfig;
  tableData: TableData;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    public globalService: GlobalService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.initTableConfig();
  }

  initTableConfig() {
    const cols: TableCols[] = [
      {
        key: 'table',
        name: this.i18n.get('common_table_label'),
        sort: true
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: false,
        virtualScroll: true,
        virtualItemHeight: 32,
        scrollFixed: true,
        scroll: { y: '420px' }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  initDetailData(data) {
    this.source = data;
    this.type = DataMap.Resource_Type.tidbTable.value;

    const tableList = JSON.parse(data.extendInfo.tableName);

    this.tableData = {
      data: map(tableList, item => {
        return {
          table: item
        };
      }),
      total: tableList.length
    };
  }
}
