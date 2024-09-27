import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-mongodb',
  templateUrl: './mongodb.component.html',
  styleUrls: ['./mongodb.component.less']
})
export class MongodbComponent implements OnInit {
  header = this.i18n.get('MongoDB');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.MongodbClusterInstance.value,
    DataMap.Resource_Type.MongodbSingleInstance.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
