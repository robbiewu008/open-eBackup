import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-resource-set-summary',
  templateUrl: './resource-set-summary.component.html',
  styleUrls: ['./resource-set-summary.component.less']
})
export class ResourceSetSummaryComponent implements OnInit {
  source;
  title = this.i18n.get('common_cloud_server_label');
  isResourceSet = false;
  dataMap = DataMap;

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}

  initDetailData(data: any) {
    this.source = data;
    this.title = this.i18n.get('common_cloud_server_label');
  }
}
