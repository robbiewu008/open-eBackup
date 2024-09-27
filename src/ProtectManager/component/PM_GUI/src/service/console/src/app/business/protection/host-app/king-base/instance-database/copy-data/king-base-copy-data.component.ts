import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-king-base-copy-data',
  templateUrl: './king-base-copy-data.component.html',
  styleUrls: ['./king-base-copy-data.component.less']
})
export class KingBaseCopyDataComponent implements OnInit {
  data;
  type;

  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.data = data;
    this.type = data.resourceType;
  }
}
