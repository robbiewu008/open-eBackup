import { Component, OnInit } from '@angular/core';
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-gaussdb-for-opengauss',
  templateUrl: './gaussdb-for-opengauss.component.html',
  styleUrls: ['./gaussdb-for-opengauss.component.less']
})
export class GaussdbForOpengaussComponent implements OnInit {
  header = this.i18n.get('protection_gaussdb_for_opengauss_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [DataMap.Resource_Type.gaussdbForOpengauss.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
