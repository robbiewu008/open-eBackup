import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-informix',
  templateUrl: './informix.component.html',
  styleUrls: ['./informix.component.less']
})
export class InformixComponent implements OnInit {
  header = this.i18n.get('Informix');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.informixInstance.value,
    DataMap.Resource_Type.informixClusterInstance.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
