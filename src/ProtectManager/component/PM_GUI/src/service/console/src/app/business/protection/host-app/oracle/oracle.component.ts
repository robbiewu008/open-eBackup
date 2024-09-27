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
import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { GlobalService, ResourceType } from 'app/shared';
import { DatabaseListComponent } from './database-list/database-list.component';
import { HostClusterListComponent } from './host-cluster-list/host-cluster-list.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { takeUntil } from 'rxjs/operators';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-oracle',
  templateUrl: './oracle.component.html',
  styleUrls: ['./oracle.component.less']
})
export class OracleComponent implements OnInit, OnDestroy {
  resourceType = ResourceType;
  activeIndex = this.resourceType.HOST;

  destroy$ = new Subject();

  @ViewChild(DatabaseListComponent, { static: false })
  databaseListComponent: DatabaseListComponent;

  @ViewChild(HostClusterListComponent, { static: false })
  hostClusterListComponent: HostClusterListComponent;

  constructor(private globalService: GlobalService) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.showGuideTab();
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        this.showGuideTab();
      });
  }

  showGuideTab() {
    if (USER_GUIDE_CACHE_DATA.active && USER_GUIDE_CACHE_DATA.activeTab) {
      setTimeout(() => {
        this.activeIndex = <any>USER_GUIDE_CACHE_DATA.activeTab;
      });
    }
  }
}
