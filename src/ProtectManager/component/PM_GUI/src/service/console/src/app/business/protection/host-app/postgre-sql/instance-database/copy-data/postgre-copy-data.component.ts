import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-postgre-copy-data',
  templateUrl: './postgre-copy-data.component.html',
  styleUrls: ['./postgre-copy-data.component.less']
})
export class PostgreCopyDataComponent implements OnInit {
  data;
  type;

  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.data = data;
    this.type = data.resourceType;
  }
}
