import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-hyperv-copy-data',
  templateUrl: './hyperv-copy-data.component.html',
  styleUrls: ['./hyperv-copy-data.component.less']
})
export class HypervCopyDataComponent implements OnInit {
  data;
  type;

  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.data = data;
    this.type = data.resourceType;
  }
}
