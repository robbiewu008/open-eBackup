import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap, I18NService } from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { includes, map } from 'lodash';

@Component({
  selector: 'aui-db-two-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  resourceType = DataMap.Resource_Type;
  subType;
  tableConfig: TableConfig;
  tableData: TableData;
  customParams = '';

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(private i18n: I18NService) {}

  ngOnInit() {
    this.initTablespaceConfig();
  }

  initDetailData(data) {
    this.source = data;
    this.subType = data.subType || data.resourceType || data.sub_type;

    if (includes([DataMap.Resource_Type.dbTwoTableSet.value], this.subType)) {
      const tableList = data.extendInfo.table.split(',');

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

  initTablespaceConfig() {
    const cols: TableCols[] = [
      {
        key: 'table',
        name: this.i18n.get('explore_database_table_space_label'),
        sort: true
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
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
  }
}
