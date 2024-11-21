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
import { DataMap } from 'app/shared';
import { InstanceDatabaseComponent } from '../../host-app/gaussdb-dws/instance-database/instance-database.component';

@Component({
  selector: 'aui-gbase',
  templateUrl: './gbase.component.html',
  styleUrls: ['./gbase.component.less']
})
export class GbaseComponent implements OnInit {
  activeIndex = 0;
  clusterConfig;
  instanceConfig;
  subType = DataMap.Resource_Type.GBase.value;

  constructor() {}

  ngOnInit(): void {
    this.initConfig();
  }

  initConfig() {
    this.clusterConfig = {
      id: DataMap.Resource_Type.gbaseCluster.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'logBackup',
        'authorizedUser',
        'operation'
      ],
      tableOpts: ['register', 'deleteResource']
    };

    this.instanceConfig = {
      id: DataMap.Resource_Type.gbaseInstance.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'clusterOrHostName',
        'version',
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
  }
}
