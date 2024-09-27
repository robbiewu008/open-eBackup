import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-saphana',
  templateUrl: './saphana.component.html',
  styleUrls: ['./saphana.component.less']
})
export class SaphanaComponent implements OnInit {
  header = this.i18n.get('protection_saphana_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.saphanaInstance.value,
    DataMap.Resource_Type.saphanaDatabase.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
