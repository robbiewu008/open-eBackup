import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-fusion-compute-copy-data',
  templateUrl: './fusion-compute-copy-data.component.html',
  styleUrls: ['./fusion-compute-copy-data.component.less']
})
export class FusionComputeCopyDataComponent implements OnInit {
  data;
  type;

  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.data = data;
    this.type = data.resourceType;
  }
}
