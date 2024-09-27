import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-hdfs',
  templateUrl: './hdfs.component.html',
  styleUrls: ['./hdfs.component.less']
})
export class HdfsComponent implements OnInit {
  header = this.i18n.get('HDFS');
  resourceType = ResourceType.BIG_DATA;
  childResourceType = [DataMap.Resource_Type.HDFSFileset.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
