import { Component, OnInit } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-kubernetes-container',
  templateUrl: './kubernetes-container.component.html',
  styleUrls: ['./kubernetes-container.component.less']
})
export class KubernetesContainerComponent implements OnInit {
  header = this.i18n.get('protection_kubernetes_container_label');
  resourceType = ResourceType.VM;
  childResourceType = [
    DataMap.Resource_Type.kubernetesNamespaceCommon.value,
    DataMap.Resource_Type.kubernetesDatasetCommon.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
