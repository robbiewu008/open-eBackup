import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-redis-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  sourceType;

  constructor() {}

  ngOnInit() {}

  initDetailData(data: any) {
    this.source = data;
    this.sourceType = data.resourceType;
  }
}
