import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { KubernetesContainerRoutingModule } from './kubernetes-container-routing.module';
import { KubernetesContainerComponent } from './kubernetes-container.component';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [KubernetesContainerComponent],
  imports: [
    CommonModule,
    KubernetesContainerRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class KubernetesContainerModule {}
