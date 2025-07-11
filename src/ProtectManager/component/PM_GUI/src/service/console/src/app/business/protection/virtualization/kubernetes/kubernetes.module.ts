/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
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
