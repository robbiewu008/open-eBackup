import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-openstack',
  templateUrl: './openstack.component.html',
  styleUrls: ['./openstack.component.less']
})
export class OpenstackComponent implements OnInit {
  header = this.i18n.get('common_open_stack_label');
  resourceType = ResourceType.OpenStack;
  childResourceType = [DataMap.Resource_Type.openStackCloudServer.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
