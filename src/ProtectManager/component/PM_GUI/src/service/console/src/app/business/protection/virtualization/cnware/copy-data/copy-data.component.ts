import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-cnware-copy-data',
  templateUrl: './copy-data.component.html',
  styleUrls: ['./copy-data.component.less']
})
export class CopyDataComponent implements OnInit {
  data;
  type;

  constructor() {}

  ngOnInit(): void {}

  initDetailData(data) {
    this.data = data;
    this.type = data.resourceType;
  }
}
