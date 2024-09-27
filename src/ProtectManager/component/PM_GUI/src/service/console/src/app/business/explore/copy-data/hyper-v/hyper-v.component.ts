import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-hyper-v',
  templateUrl: './hyper-v.component.html',
  styleUrls: ['./hyper-v.component.less']
})
export class HyperVComponent implements OnInit {
  header = this.i18n.get('common_hyperv_label');
  resourceType = ResourceType.Virtualization;
  childResourceType = [
    DataMap.Resource_Type.hyperVVm.value,
    DataMap.Resource_Type.hyperVHost.value,
    DataMap.Resource_Type.hyperVCluster.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit() {}
}
