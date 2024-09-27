import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService } from 'app/shared';
import { TableCols } from 'app/shared/components/pro-table';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import { split, map as _map, set as _set, join } from 'lodash';

@Component({
  selector: 'aui-hbase-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  showTree = true;
  tableConfig;
  tableData;

  constructor(
    private i18n: I18NService,
    private virtualScroll: VirtualScrollService
  ) {}

  ngOnInit() {}

  initDetailData(data) {
    data.sub_type = DataMap.Resource_Type.HBaseBackupSet.value;

    if (data.type === DataMap.Resource_Type.Hive.value) {
      const tableList = split(data.extendInfo.tableList, ',');
      const table = _map(tableList, value => {
        return value
          ? `/${data.extendInfo.databaseName}/${value}`
          : `/${data.extendInfo.databaseName}`;
      });

      this.showTree = !data.extendInfo?.tableList;
      data.sub_type = DataMap.Resource_Type.HiveBackupSet.value;
      _set(data, 'extendInfo.table', join(table, ','));
    }

    if (data.type === DataMap.Resource_Type.Elasticsearch.value) {
      if (data.extendInfo.indexList !== '*') {
        const tableList = split(data.extendInfo.indexList, ',');
        const table = _map(tableList, value => {
          return value
            ? `/${data.environment?.name || data?.environment_name}/${value}`
            : `/${data.environment?.name || data?.environment_name}`;
        });

        this.showTree = false;
        _set(data, 'extendInfo.table', join(table, ','));
      } else {
        const table = [`/${data.environment?.name || data?.environment_name}`];
        this.showTree = true;
        _set(data, 'extendInfo.table', join(table, ','));
      }
      data.sub_type = DataMap.Resource_Type.ElasticsearchBackupSet.value;
    }
    this.source = data;

    if (!this.showTree) {
      const tableData =
        data.type === DataMap.Resource_Type.Elasticsearch.value
          ? data.extendInfo.indexList
          : data.extendInfo.tableList;
      this.tableData = {
        data: _map(split(tableData, ','), val => {
          return {
            table: val
          };
        }),
        total: split(tableData, ',').length
      };
      this.initConfig();
    }
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'table',
        name: this.i18n.get('common_table_label')
      }
    ];

    this.tableConfig = {
      table: {
        async: false,
        compareWith: 'table',
        columns: cols,
        virtualScroll: true,
        scrollFixed: true,
        scroll: { y: '580px' },
        colDisplayControl: false
      }
    };
  }
}
