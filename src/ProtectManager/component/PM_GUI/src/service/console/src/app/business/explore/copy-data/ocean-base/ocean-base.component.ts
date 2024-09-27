import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-ocean-base',
  templateUrl: './ocean-base.component.html',
  styleUrls: ['./ocean-base.component.less']
})
export class OceanBaseComponent implements OnInit {
  header = this.i18n.get('OceanBase');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.OceanBase.value,
    DataMap.Resource_Type.OceanBaseCluster.value,
    DataMap.Resource_Type.OceanBaseTenant.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
