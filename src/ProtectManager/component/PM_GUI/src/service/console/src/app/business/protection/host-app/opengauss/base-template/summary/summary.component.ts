import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-opengauss-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  constructor() {}

  ngOnInit(): void {}

  initDetailData(data) {
    this.source = data;
  }
}
