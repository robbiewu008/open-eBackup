import { Component, OnInit } from '@angular/core';
import { get, isUndefined } from 'lodash';
@Component({
  selector: 'aui-select-copy-resource',
  templateUrl: './select-copy-resource.component.html',
  styleUrls: ['./select-copy-resource.component.less']
})
export class SelectCopyResourceComponent implements OnInit {
  resourceData;
  disableSLA = false;
  constructor() {}

  ngOnInit() {
    this.disableSLA =
      !isUndefined(get(this.resourceData, 'disableSla')) &&
      this.resourceData.disableSla;
  }

  initData(data: any) {
    this.resourceData = data;
  }

  onOK() {
    return { selectedList: [] };
  }
}
