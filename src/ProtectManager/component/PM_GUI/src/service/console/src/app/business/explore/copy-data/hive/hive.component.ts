import { Component, OnInit } from '@angular/core';
import { I18NService, DataMap, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-hive',
  templateUrl: './hive.component.html',
  styleUrls: ['./hive.component.less']
})
export class HiveComponent implements OnInit {
  header = this.i18n.get('Hive');
  resourceType = ResourceType.BIG_DATA;
  childResourceType = [DataMap.Resource_Type.HiveBackupSet.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
