import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap } from 'app/shared';
import {
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign } from 'lodash';

@Component({
  selector: 'aui-gaussdb-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  resourceType = DataMap.Resource_Type;
  subType = DataMap.Resource_Type.gaussdbForOpengauss.value;
  tableConfig: TableConfig;
  tableData: TableData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.source = assign(data, {
      subType: data.sub_type || data.subType
    });
    this.subType = data.sub_type || data.subType;
  }
}
