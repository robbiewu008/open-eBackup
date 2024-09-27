import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';
import { assign } from 'lodash';

@Component({
  selector: 'aui-mysql-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  resourceType = DataMap.Resource_Type;
  subType = DataMap.Resource_Type.MySQL.value;

  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.source = assign(data, {
      subType: data.sub_type || data?.subType
    });
    this.subType = data.sub_type || data.subType;
  }
}
