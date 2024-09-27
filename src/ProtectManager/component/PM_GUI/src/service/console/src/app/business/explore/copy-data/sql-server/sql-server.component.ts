import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap } from 'app/shared';

@Component({
  selector: 'aui-sql-server',
  templateUrl: './sql-server.component.html',
  styleUrls: ['./sql-server.component.less']
})
export class SQLServerComponent implements OnInit {
  header = DataMap.Resource_Type.SQLServer.label;
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.SQLServerCluster.value,
    DataMap.Resource_Type.SQLServerInstance.value,
    DataMap.Resource_Type.SQLServerClusterInstance.value,
    DataMap.Resource_Type.SQLServerGroup.value,
    DataMap.Resource_Type.SQLServerDatabase.value
  ];

  constructor() {}

  ngOnInit(): void {}
}
