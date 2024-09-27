import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-copy-data-opengauss',
  templateUrl: './copy-data.component.html',
  styleUrls: ['./copy-data.component.less']
})
export class CopyDataComponent implements OnInit {
  data;
  resourceType;

  constructor() {}

  ngOnInit(): void {}

  initDetailData(data: any) {
    this.data = data;
    this.resourceType = data.subType || data.sub_type;
  }
}
