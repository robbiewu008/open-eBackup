import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap } from 'app/shared';

@Component({
  selector: 'aui-click-house',
  templateUrl: './click-house.component.html',
  styleUrls: ['./click-house.component.css']
})
export class ClickHouseComponent implements OnInit {
  header = 'ClickHouse';
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.ClickHouse.value,
    DataMap.Resource_Type.ClickHouseCluster.value,
    DataMap.Resource_Type.ClickHouseDatabase.value,
    DataMap.Resource_Type.ClickHouseTableset.value
  ];

  constructor() {}

  ngOnInit() {}
}
