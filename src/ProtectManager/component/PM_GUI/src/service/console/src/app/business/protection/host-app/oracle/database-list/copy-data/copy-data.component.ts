import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap } from 'app/shared';

@Component({
  selector: 'aui-database-copy-data',
  templateUrl: './copy-data.component.html',
  styleUrls: ['./copy-data.component.less']
})
export class CopyDataComponent implements OnInit {
  data;
  type = DataMap.Resource_Type.oracle.value;

  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.data = data;
  }
}
