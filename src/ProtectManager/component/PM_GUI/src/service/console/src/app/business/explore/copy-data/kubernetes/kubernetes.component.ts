import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-kubernetes',
  templateUrl: './kubernetes.component.html',
  styleUrls: ['./kubernetes.component.less']
})
export class KubernetesComponent implements OnInit {
  header = this.i18n.get('protection_kubernetes_flexvolume_label');
  resourceType = ResourceType.VM;
  childResourceType = [DataMap.Resource_Type.KubernetesStatefulset.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
