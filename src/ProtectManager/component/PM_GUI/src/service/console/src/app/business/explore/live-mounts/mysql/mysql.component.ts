import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-mysql',
  templateUrl: './mysql.component.html',
  styleUrls: ['./mysql.component.less']
})
export class MysqlComponent implements OnInit {
  header = this.i18n.get('protection_mysql_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [DataMap.Resource_Type.MySQLInstance.value];

  constructor(private i18n: I18NService) {}

  ngOnInit() {}
}
