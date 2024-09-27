import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';
import { assign } from 'lodash';

@Component({
  selector: 'aui-summary-commonshare',
  templateUrl: './summary-commonshare.component.html',
  styleUrls: ['./summary-commonshare.component.less']
})
export class SummaryCommonShareComponent implements OnInit {
  source;
  subType = DataMap.Resource_Type.commonShare.value;
  ips = [];
  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.source = assign(data, {
      subType: data.sub_type || data?.subType
    });
    this.subType = data.sub_type || data.subType;
  }
}
