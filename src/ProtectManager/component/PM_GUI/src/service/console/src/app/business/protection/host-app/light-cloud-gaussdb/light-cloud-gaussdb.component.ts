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
import { RegisterComponent } from './register/register.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { takeUntil } from 'rxjs/operators';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-light-cloud-gaussdb',
  templateUrl: './light-cloud-gaussdb.component.html',
  styleUrls: ['./light-cloud-gaussdb.component.less']
})
export class LightCloudGaussdbComponent implements OnInit, OnDestroy {
  activeIndex = 0;
  instanceConfig;
  clusterConfig;

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
      activeIndex: DataMap.Resource_Type.lightCloudGaussdbProject.value,
      tableCols: [
        'uuid',
        'name',
        'pmAddress',
        'linkStatus',
        'tpopsVersion',
        'isAllowRestore',
        'authorizedUser',
        'operation'
      ],
      tableOpts: [
        'register',
        'resourceAuth',
        'resourceReclaiming',
        'rescan',
        'connectivityTest',
        'allowRecovery',
        'disableRecovery',
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterComponent
    };

    this.instanceConfig = {
      activeIndex: DataMap.Resource_Type.lightCloudGaussdbInstance.value,
      tableCols: [
        'uuid',
        'name',
        'pmAddress',
        'instanceStatus',
        'parentName',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'isAllowRestore',
        'operation'
      ],
      tableOpts: [
        'protect',
        'modifyProtect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'recovery',
        'allowRecovery',
        'disableRecovery',
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
