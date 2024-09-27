import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-vmware',
  templateUrl: './vmware.component.html',
  styleUrls: ['./vmware.component.less']
})
export class VmwareComponent implements OnInit {
  header = this.i18n.get('common_vmware_label');
  resourceType = ResourceType.VM;
  childResourceType = [DataMap.Resource_Type.virtualMachine.value];

  constructor(private i18n: I18NService) {}

  ngOnInit() {}
}
