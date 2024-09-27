import { Component, OnInit } from '@angular/core';
import { DataMap, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-apsara-stack',
  templateUrl: './apsara-stack.component.html',
  styleUrls: ['./apsara-stack.component.less']
})
export class ApsaraStackComponent implements OnInit {
  header = 'ApsaraStack';
  resourceType = ResourceType.ApsaraStack;
  childResourceType = [
    DataMap.Resource_Type.APSResourceSet.value,
    DataMap.Resource_Type.APSCloudServer.value,
    DataMap.Resource_Type.APSZone.value
  ];

  constructor() {}

  ngOnInit(): void {}
}
