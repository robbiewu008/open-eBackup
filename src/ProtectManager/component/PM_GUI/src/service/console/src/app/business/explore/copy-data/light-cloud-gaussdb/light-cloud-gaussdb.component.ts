import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-light-cloud-gaussdb',
  templateUrl: './light-cloud-gaussdb.component.html',
  styleUrls: ['./light-cloud-gaussdb.component.less']
})
export class LightCloudGaussdbComponent implements OnInit {
  header = this.i18n.get('protection_light_cloud_gaussdb_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [DataMap.Resource_Type.lightCloudGaussdbInstance.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
