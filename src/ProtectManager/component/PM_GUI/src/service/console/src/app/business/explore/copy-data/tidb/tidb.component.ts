import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap } from 'app/shared';

@Component({
  selector: 'aui-tidb',
  templateUrl: './tidb.component.html',
  styleUrls: ['./tidb.component.less']
})
export class TidbComponent implements OnInit {
  header = DataMap.Resource_Type.tidb.label;
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.tidb.value,
    DataMap.Resource_Type.tidbCluster.value,
    DataMap.Resource_Type.tidbDatabase.value,
    DataMap.Resource_Type.tidbTable.value
  ];

  constructor() {}

  ngOnInit(): void {}
}
