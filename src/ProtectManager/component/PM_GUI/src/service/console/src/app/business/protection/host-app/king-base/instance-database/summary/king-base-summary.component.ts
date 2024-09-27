import { Component, OnInit, ViewChild } from '@angular/core';
import { DataMap } from 'app/shared';
import { KingBaseProxyHostComponent } from './proxy-host/king-base-proxy-host.component';
@Component({
  selector: 'aui-king-base-summary',
  templateUrl: './king-base-summary.component.html',
  styleUrls: ['./king-base-summary.component.less']
})
export class KingBaseSummaryComponent implements OnInit {
  source;
  subType;
  dataMap = DataMap;
  constructor() {}

  @ViewChild(KingBaseProxyHostComponent, { static: false })
  instanceDatabaseComponent: KingBaseProxyHostComponent;
  ngOnInit() {}

  initDetailData(data) {
    this.source = data;
    this.subType = data.sub_type || data?.subType;
  }

  update() {
    this.instanceDatabaseComponent.refresh();
  }
}
