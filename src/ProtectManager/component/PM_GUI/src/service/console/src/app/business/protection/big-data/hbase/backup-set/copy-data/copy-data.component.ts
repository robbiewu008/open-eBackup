import { DataMap } from 'app/shared';
import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-backup-set-copy-data',
  templateUrl: './copy-data.component.html',
  styleUrls: ['./copy-data.component.less']
})
export class CopyDataComponent implements OnInit {
  data;

  constructor() {}

  ngOnInit() {}

  initDetailData(data: any) {
    this.data = data;
  }
}
