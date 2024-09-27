import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap } from 'app/shared';

@Component({
  selector: 'aui-postgre-sql',
  templateUrl: './postgre-sql.component.html',
  styleUrls: ['./postgre-sql.component.less']
})
export class PostgreSQLComponent implements OnInit {
  header = 'PostgreSQL';
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.PostgreSQLInstance.value,
    DataMap.Resource_Type.PostgreSQLClusterInstance.value
  ];

  constructor() {}

  ngOnInit(): void {}
}
