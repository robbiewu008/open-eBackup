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
import { InstanceDatabaseComponent } from '../gaussdb-dws/instance-database/instance-database.component';

@Component({
  selector: 'aui-gaussdb-for-opengauss',
  templateUrl: './gaussdb-for-opengauss.component.html',
  styleUrls: ['./gaussdb-for-opengauss.component.less']
})
export class GaussdbForOpengaussComponent implements OnInit {
  activeIndex = 0;
  instanceConfig;
  clusterConfig;
  subType = DataMap.Resource_Type.gaussdbForOpengauss.value;

  @ViewChild(InstanceDatabaseComponent, { static: false })
  instanceDatabaseComponent: InstanceDatabaseComponent;

  constructor() {}

  ngOnInit() {
    this.clusterConfig = {
      id: DataMap.Resource_Type.gaussdbForOpengaussProject.value,
      tableCols: ['uuid', 'name', 'linkStatus', 'authorizedUser', 'operation'],
      tableOpts: ['register', 'deleteResource']
    };

    this.instanceConfig = {
      id: DataMap.Resource_Type.gaussdbForOpengaussInstance.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'ownedProject',
        'region',
        'version',
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
  }

  onChange() {
    this.instanceDatabaseComponent.ngOnInit();
  }
}
