import { Component, OnInit, ViewChild } from '@angular/core';
import { ClusterComponent } from './cluster/cluster.component';
import { NamespaceComponent } from './namespace/namespace.component';
import { StatefulsetComponent } from './statefulset/statefulset.component';

@Component({
  selector: 'aui-kubernetes',
  templateUrl: './kubernetes.component.html',
  styleUrls: ['./kubernetes.component.less']
})
export class KubernetesComponent implements OnInit {
  activeIndex = 'cluster';
  constructor() {}

  @ViewChild(ClusterComponent, { static: false })
  ClusterComponent: ClusterComponent;

  @ViewChild(NamespaceComponent, { static: false })
  NamespaceComponent: NamespaceComponent;

  @ViewChild(StatefulsetComponent, { static: false })
  StatefulsetComponent: StatefulsetComponent;

  ngOnInit() {}

  onChange() {
    if (this.activeIndex === 'cluster') {
      this.ClusterComponent.ngOnInit();
    } else if (this.activeIndex === 'namespace') {
      this.NamespaceComponent.ngOnInit();
    } else {
      this.StatefulsetComponent.ngOnInit();
    }
  }
}
