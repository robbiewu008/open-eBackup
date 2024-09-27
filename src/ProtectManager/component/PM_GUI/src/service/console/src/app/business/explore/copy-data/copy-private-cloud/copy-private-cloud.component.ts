import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-copy-private-cloud',
  templateUrl: './copy-private-cloud.component.html',
  styleUrls: ['./copy-private-cloud.component.less']
})
export class CopyPrivateCloudComponent implements OnInit {
  header = this.i18n.get('common_cloud_server_label');
  resourceType = ResourceType.HCS;
  childResourceType = [DataMap.Resource_Type.HCSCloudHost.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
