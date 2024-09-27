import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { KubernetesContainerRoutingModule } from './kubernetes-container-routing.module';
import { KubernetesContainerComponent } from './kubernetes-container.component';
import { ClusterModule } from '../kubernetes/cluster/cluster.module';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { BaseTemplateModule } from '../kubernetes/base-template/base-template.module';
import { RegisterDatasetModule } from './register-dataset/register-dataset.module';

@NgModule({
  declarations: [KubernetesContainerComponent],
  imports: [
    CommonModule,
    KubernetesContainerRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    ClusterModule,
    BaseTemplateModule,
    RegisterDatasetModule
  ],
  exports: [KubernetesContainerComponent]
})
export class KubernetesContainerModule {}
