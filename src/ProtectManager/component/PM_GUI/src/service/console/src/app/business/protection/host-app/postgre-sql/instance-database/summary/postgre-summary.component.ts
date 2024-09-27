import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-summary-instance-database',
  templateUrl: './postgre-summary.component.html',
  styleUrls: ['./postgre-summary.component.less']
})
export class PostgreSummaryComponent implements OnInit {
  source;
  subType;
  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.source = data;
    this.subType = data.sub_type;
  }
}
