import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { KubernetesRoutingModule } from './kubernetes-routing.module';
import { KubernetesComponent } from './kubernetes.component';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [KubernetesComponent],
  imports: [
    CommonModule,
    KubernetesRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class KubernetesModule {}
