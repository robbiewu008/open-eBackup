import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';
import { assign } from 'lodash';

@Component({
  selector: 'aui-ad-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  data;
  sourceType = DataMap.Resource_Type.ActiveDirectory.value;
  constructor() {}

  ngOnInit(): void {}

  initDetailData(data, config) {
    this.source = data;
    assign(this.source, {
      linkStatus: data.environment.linkStatus
    });
  }
}
