import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-cnware',
  templateUrl: './cnware.component.html',
  styleUrls: ['./cnware.component.less']
})
export class CnwareComponent implements OnInit {
  header = this.i18n.get('common_cnware_label');
  resourceType = ResourceType.VM;
  childResourceType = [DataMap.Resource_Type.cNwareVm.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
