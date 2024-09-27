import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-hcs-copy-data',
  templateUrl: './hcs-copy-data.component.html',
  styleUrls: ['./hcs-copy-data.component.less']
})
export class HCSCopyDataComponent implements OnInit {
  data;
  type;

  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.data = data;
    this.type = data.subType || DataMap.Resource_Type.HCSCloudHost.value;
  }
}
