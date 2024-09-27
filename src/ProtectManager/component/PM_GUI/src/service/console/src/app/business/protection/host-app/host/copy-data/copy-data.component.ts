import { Component, OnInit } from '@angular/core';
import { DataMap } from './../../../../../shared/consts/data-map.config';

@Component({
  selector: 'aui-host-copy-data',
  templateUrl: './copy-data.component.html'
})
export class CopyDataComponent implements OnInit {
  data;
  resourceType = DataMap.Resource_Type.ABBackupClient.value;

  constructor() {}

  ngOnInit() {}

  initDetailData(data: any) {
    this.data = data;
  }
}
