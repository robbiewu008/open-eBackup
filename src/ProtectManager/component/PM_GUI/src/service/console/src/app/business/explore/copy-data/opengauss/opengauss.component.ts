import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-opengauss',
  templateUrl: './opengauss.component.html',
  styleUrls: ['./opengauss.component.less']
})
export class OpengaussComponent implements OnInit {
  header = this.i18n.get('openGauss');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.OpenGauss.value,
    DataMap.Resource_Type.OpenGauss_database.value,
    DataMap.Resource_Type.OpenGauss_instance.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
