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
import { Component, OnInit } from '@angular/core';
import { RegisterComponent } from './register/register.component';
import { TemplateParams } from 'app/business/protection/host-app/database-template/database-template.component';
import { DataMap, GlobalService } from 'app/shared';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-ant-db',
  templateUrl: './ant-db.component.html',
  styleUrls: ['./ant-db.component.less']
})
export class AntDBComponent implements OnInit {
  activeIndex = DataMap.Resource_Type.AntDBClusterInstance.value;
  dataMap = DataMap;
  destroy$ = new Subject();
  clusterConfig: TemplateParams;
  instanceConfig: TemplateParams;

  constructor(private globalService: GlobalService) {}

  ngOnInit() {
    this.instanceConfig = {
      activeIndex: DataMap.Resource_Type.AntDB.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'subType',
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
      registerComponent: RegisterComponent
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
        this.activeIndex = USER_GUIDE_CACHE_DATA.activeTab;
      });
    }
  }
}
