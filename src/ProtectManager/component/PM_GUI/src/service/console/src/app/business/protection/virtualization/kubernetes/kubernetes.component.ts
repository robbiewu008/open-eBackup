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
