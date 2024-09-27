import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap } from 'app/shared';

@Component({
  selector: 'aui-tdsql',
  templateUrl: './tdsql.component.html',
  styleUrls: ['./tdsql.component.less']
})
export class TdsqlComponent implements OnInit {
  header = DataMap.Resource_Type.tdsqlInstance.label;
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.tdsqlInstance.value,
    DataMap.Resource_Type.tdsqlDistributedInstance.value
  ];

  constructor() {}

  ngOnInit(): void {}
}
