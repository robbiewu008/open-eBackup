import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-dameng',
  templateUrl: './dameng.component.html',
  styleUrls: ['./dameng.component.less']
})
export class DamengComponent implements OnInit {
  header = this.i18n.get('Dameng');
  resourceType = ResourceType.DATABASE;

  childResourceType = [
    DataMap.Resource_Type.Dameng.value,
    DataMap.Resource_Type.Dameng_cluster.value,
    DataMap.Resource_Type.Dameng_singleNode.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
