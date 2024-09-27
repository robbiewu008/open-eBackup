import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-gaussdb-t',
  templateUrl: './gaussdb-t.component.html',
  styleUrls: ['./gaussdb-t.component.less']
})
export class GaussdbTComponent implements OnInit {
  header = 'GaussDB T';
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.GaussDB_T.value,
    DataMap.Resource_Type.gaussdbTSingle.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit() {}
}
