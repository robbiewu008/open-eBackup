import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-oracle',
  templateUrl: './oracle.component.html',
  styleUrls: ['./oracle.component.less']
})
export class OracleComponent implements OnInit {
  header = this.i18n.get('common_oracle_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.oracle.value,
    DataMap.Resource_Type.oracleCluster.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit() {}
}
