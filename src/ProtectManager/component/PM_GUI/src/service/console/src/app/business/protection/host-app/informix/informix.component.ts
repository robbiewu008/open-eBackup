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
import { DataMap, GlobalService } from 'app/shared';
import { DatabaseTemplateComponent } from '../database-template/database-template.component';
import { RegisterClusterComponent } from './register-cluster/register-cluster.component';
import { RegisterInstanceComponent } from './register-instance/register-instance.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';
@Component({
  selector: 'aui-informix',
  templateUrl: './informix.component.html',
  styleUrls: ['./informix.component.less']
})
export class InformixComponent implements OnInit, OnDestroy {
  activeIndex = 0;
  clusterConfig;
  instanceConfig;

  destroy$ = new Subject();

  @ViewChild(DatabaseTemplateComponent, { static: false })
  databaseTemplateComponent: DatabaseTemplateComponent;

  constructor(private globalService: GlobalService) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit() {
    this.clusterConfig = {
      activeIndex: DataMap.Resource_Type.informixService.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'logBackup',
        'authorizedUser',
        'operation'
      ],
      tableOpts: [
        'register',
        'resourceAuth',
        'resourceReclaiming',
        'connectivityTest',
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterClusterComponent
    };

    this.instanceConfig = {
      activeIndex: DataMap.Resource_Type.informixInstance.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'environmentName',
        'databaseType',
        'version',
        'sla_name',
        'sla_compliance',
        'protectionStatus',
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
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterInstanceComponent
    };

    this.showGuideTab();
    this.globalService
      .getState(USER_GUIDE_CACHE_DATA.action)
      .pipe(takeUntil(this.destroy$))
      .subscribe(() => {
        this.showGuideTab();
      });
  }

  onChange() {
    this.databaseTemplateComponent.ngOnInit();
  }

  showGuideTab() {
    if (USER_GUIDE_CACHE_DATA.active && USER_GUIDE_CACHE_DATA.activeTab) {
      setTimeout(() => {
        this.activeIndex = <any>USER_GUIDE_CACHE_DATA.activeTab;
      });
    }
  }
}
