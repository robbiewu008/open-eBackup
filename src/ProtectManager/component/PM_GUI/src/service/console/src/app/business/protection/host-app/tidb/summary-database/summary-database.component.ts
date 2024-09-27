import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import { TableConfig, TableData } from 'app/shared/components/pro-table';
import {
  toString,
  assign,
  defer,
  size,
  map,
  isEmpty,
  each,
  find
} from 'lodash';

@Component({
  selector: 'aui-summary-database',
  templateUrl: './summary-database.component.html',
  styleUrls: ['./summary-database.component.less']
})
export class SummaryDatabaseComponent implements OnInit {
  source;
  dbInfo;
  type = DataMap.Resource_Type.tidbDatabase.value;
  dataMap = DataMap;

  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('instNameTpl', { static: true }) instNameTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {}

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
}
