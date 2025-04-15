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
import { Component, OnDestroy, OnInit } from '@angular/core';
import { DataMap, GlobalService } from 'app/shared';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-db-two',
  templateUrl: './db-two.component.html',
  styleUrls: ['./db-two.component.less']
})
export class DbTwoComponent implements OnInit, OnDestroy {
  activeIndex = 0;
  clusterConfig;
  instanceConfig;
  groupConfig;
  databaseConfig;
  subType = DataMap.Resource_Type.dbTwoCluster.value;

  destroy$ = new Subject();

  constructor(private globalService: GlobalService) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.clusterConfig = {
      id: DataMap.Resource_Type.dbTwoCluster.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'clusterType',
        'authorizedUser',
        'operation'
      ],
      tableOpts: ['register', 'deleteResource']
    };

    this.instanceConfig = {
      id: DataMap.Resource_Type.dbTwoInstance.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'clusterOrHostName',
        'version',
        'operation'
      ],
      tableOpts: ['register', 'deleteResource']
    };

    this.groupConfig = {
      id: DataMap.Resource_Type.dbTwoDatabase.value,
      tableCols: [
        'uuid',
        'name',
        'ownedInstance',
        'clusterOrHostName',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'operation'
      ],
      tableOpts: [
        'protect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'manualBackup'
      ]
    };

    this.databaseConfig = {
      id: DataMap.Resource_Type.dbTwoTableSet.value,
      tableCols: [
        'uuid',
        'name',
        'clusterOrHostName',
        'ownedInstance',
        'database',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'operation'
      ],
      tableOpts: [
        'register',
        'protect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'manualBackup',
        'deleteResource'
      ]
    };

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
