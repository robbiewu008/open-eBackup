import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { KubernetesRoutingModule } from './kubernetes-routing.module';
import { KubernetesComponent } from './kubernetes.component';
import { BaseModule } from 'app/shared';
import { ClusterModule } from './cluster/cluster.module';
import { NamespaceModule } from './namespace/namespace.module';
import { StatefulsetModule } from './statefulset/statefulset.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { AdvancedParameterModule } from './base-template/advanced-parameter/advanced-parameter.module';

@NgModule({
  declarations: [KubernetesComponent],
  imports: [
    CommonModule,
    KubernetesRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    ClusterModule,
    NamespaceModule,
    StatefulsetModule,
    AdvancedParameterModule
  ],
  exports: [KubernetesComponent]
})
export class KubernetesModule {}
