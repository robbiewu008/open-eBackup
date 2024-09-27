import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-redis-copy-data',
  templateUrl: './redis-copy-data.component.html',
  styleUrls: ['./redis-copy-data.component.less']
})
export class RedisCopyDataComponent implements OnInit {
  data;
  type;

  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.data = data;
    this.type = data.resourceType;
  }
}
