import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap } from 'app/shared';

@Component({
  selector: 'aui-king-base',
  templateUrl: './king-base.component.html',
  styleUrls: ['./king-base.component.less']
})
export class KingBaseComponent implements OnInit {
  header = 'Kingbase';
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.KingBaseInstance.value,
    DataMap.Resource_Type.KingBaseClusterInstance.value
  ];

  constructor() {}

  ngOnInit(): void {}
}
