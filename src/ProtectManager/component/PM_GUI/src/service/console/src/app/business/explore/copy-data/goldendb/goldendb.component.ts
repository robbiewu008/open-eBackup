import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-goldendb',
  templateUrl: './goldendb.component.html',
  styleUrls: ['./goldendb.component.less']
})
export class GoldendbComponent implements OnInit {
  header = this.i18n.get('protection_goldendb_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [DataMap.Resource_Type.goldendbInstance.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
