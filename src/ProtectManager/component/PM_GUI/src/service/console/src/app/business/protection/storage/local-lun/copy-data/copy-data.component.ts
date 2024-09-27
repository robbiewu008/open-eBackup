import { Component } from '@angular/core';

@Component({
  selector: 'aui-copy-data-lun',
  templateUrl: './copy-data.component.html',
  styleUrls: ['./copy-data.component.less']
})
export class CopyDataComponent {
  data;
  type;

  constructor() {}

  ngOnInit() {}

  initDetailData(data) {
    this.data = data;
    this.type = data.resourceType;
  }
}
