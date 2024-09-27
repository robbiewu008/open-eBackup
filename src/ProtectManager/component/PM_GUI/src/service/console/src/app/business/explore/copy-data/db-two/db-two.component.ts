import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-db-two',
  templateUrl: './db-two.component.html',
  styleUrls: ['./db-two.component.less']
})
export class DbTwoComponent implements OnInit {
  header = this.i18n.get('protection_db_two_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.dbTwoDatabase.value,
    DataMap.Resource_Type.dbTwoTableSet.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
