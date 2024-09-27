import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-gaussdb-dws',
  templateUrl: './gaussdb-dws.component.html',
  styleUrls: ['./gaussdb-dws.component.less']
})
export class GaussDBDWSComponent implements OnInit {
  header = this.i18n.get('common_dws_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.DWS_Cluster.value,
    DataMap.Resource_Type.DWS_Database.value,
    DataMap.Resource_Type.DWS_Schema.value,
    DataMap.Resource_Type.DWS_Table.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
