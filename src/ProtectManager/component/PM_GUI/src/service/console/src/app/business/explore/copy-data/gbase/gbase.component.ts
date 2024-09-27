import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-gbase',
  templateUrl: './gbase.component.html',
  styleUrls: ['./gbase.component.less']
})
export class GbaseComponent implements OnInit {
  header = this.i18n.get('protection_gbase_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.gbaseClusterInstance.value,
    DataMap.Resource_Type.gbaseInstance.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
