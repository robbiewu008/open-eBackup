import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-fileset-copy-data',
  templateUrl: './copy-data.component.html'
})
export class CopyDataComponent implements OnInit {
  data;
  resourceType = DataMap.Resource_Type.fileset.value;

  constructor() {}

  ngOnInit() {}

  initDetailData(data: any) {
    this.data = data;
  }
}
