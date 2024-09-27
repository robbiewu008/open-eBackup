import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-tdsql',
  templateUrl: './tdsql.component.html',
  styleUrls: ['./tdsql.component.less']
})
export class TdsqlComponent implements OnInit {
  header = this.i18n.get('TDSQL');
  resourceType = ResourceType.DATABASE;
  childResourceType = [DataMap.Resource_Type.tdsqlInstance.value];

  constructor(private i18n: I18NService) {}

  ngOnInit() {}
}
