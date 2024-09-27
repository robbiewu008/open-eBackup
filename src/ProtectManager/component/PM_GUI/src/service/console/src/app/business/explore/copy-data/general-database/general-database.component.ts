import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-general-database',
  templateUrl: './general-database.component.html',
  styleUrls: ['./general-database.component.less']
})
export class GeneralDatabaseComponent implements OnInit {
  header = this.i18n.get('protection_general_database_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [DataMap.Resource_Type.generalDatabase.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
