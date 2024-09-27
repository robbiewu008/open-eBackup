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
import { RegisterClusterComponent } from './register-cluster/register-cluster.component';
import { RegisterDatabaseComponent } from './register-database/register-database.component';
import { RegisterTableComponent } from './register-table/register-table.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-tidb',
  templateUrl: './tidb.component.html',
  styleUrls: ['./tidb.component.less']
})
export class TidbComponent implements OnInit, OnDestroy {
  activeIndex = DataMap.Resource_Type.tidbCluster.value;
  clusterConfig;
  databaseConfig;
  tableConfig;

  destroy$ = new Subject();

  constructor(private globalService: GlobalService) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.clusterConfig = {
      activeIndex: DataMap.Resource_Type.tidbCluster.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'version',
        'sla_name',
        'sla_compliance',
        'protectionStatus',
        'authorizedUser',
        'operation'
      ],
      tableOpts: [
        'register',
        'protect',
        'modifyProtect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'recovery',
        'manualBackup',
        'connectivityTest',
        'deleteResource',
        'modify'
      ],
      registerComponent: RegisterClusterComponent
    };

    this.databaseConfig = {
      activeIndex: DataMap.Resource_Type.tidbDatabase.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'sla_name',
        'sla_compliance',
        'protectionStatus',
        'operation',
        'parentName'
      ],
      tableOpts: [
        'register',
        'protect',
        'modifyProtect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'recovery',
        'manualBackup',
        'connectivityTest',
        'deleteResource',
        'modify'
      ],
      registerComponent: RegisterDatabaseComponent
    };

    this.tableConfig = {
      activeIndex: DataMap.Resource_Type.tidbTable.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'sla_name',
        'sla_compliance',
        'protectionStatus',
        'operation',
        'parentName',
        'environmentName'
      ],
      tableOpts: [
        'register',
        'protect',
        'modifyProtect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'recovery',
        'manualBackup',
        'connectivityTest',
        'deleteResource',
        'modify'
      ],
      registerComponent: RegisterTableComponent
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
