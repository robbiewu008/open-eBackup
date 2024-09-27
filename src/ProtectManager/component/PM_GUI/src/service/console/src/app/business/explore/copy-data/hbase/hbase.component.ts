import { Component, OnInit } from '@angular/core';
import { I18NService, DataMap, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-hbase',
  templateUrl: './hbase.component.html',
  styleUrls: ['./hbase.component.less']
})
export class HbaseComponent implements OnInit {
  header = this.i18n.get('HBase');
  resourceType = ResourceType.BIG_DATA;
  childResourceType = [DataMap.Resource_Type.HBaseBackupSet.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
