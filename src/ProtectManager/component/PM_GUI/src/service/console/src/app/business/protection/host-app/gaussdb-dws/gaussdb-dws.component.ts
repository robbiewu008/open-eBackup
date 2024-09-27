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
import { InstanceDatabaseComponent } from './instance-database/instance-database.component';

@Component({
  selector: 'aui-gaussdb-dws',
  templateUrl: './gaussdb-dws.component.html',
  styleUrls: ['./gaussdb-dws.component.less']
})
export class GaussdbDWSComponent implements OnInit {
  activeIndex = 0;
  clusterConfig;
  databaseConfig;
  schemaConfig;
  tableConfig;
  subType = DataMap.Resource_Type.DWS_Cluster.value;
  optItem = [
    'register',
    'protect',
    'removeProtection',
    'activeProtection',
    'deactiveProtection',
    'allowRecovery',
    'disableRecovery',
    'manualBackup',
    'deleteResource'
  ];

  @ViewChild(InstanceDatabaseComponent, { static: false })
  instanceDatabaseComponent: InstanceDatabaseComponent;

  constructor() {}

  ngOnInit() {
    this.clusterConfig = {
      id: DataMap.Resource_Type.DWS_Cluster.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'version',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'isAllowRestore',
        'authorizedUser',
        'operation'
      ],
      tableOpts: this.optItem
    };

    this.databaseConfig = {
      id: DataMap.Resource_Type.DWS_Database.value,
      tableCols: [
        'uuid',
        'name',
        'clusterOrHostName',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'isAllowRestore',
        'operation'
      ],
      tableOpts: [
        'protect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'allowRecovery',
        'disableRecovery',
        'manualBackup'
      ]
    };

    this.schemaConfig = {
      id: DataMap.Resource_Type.DWS_Schema.value,
      tableCols: [
        'uuid',
        'name',
        'clusterOrHostName',
        'database',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'isAllowRestore',
        'operation'
      ],
      tableOpts: this.optItem
    };

    this.tableConfig = {
      id: DataMap.Resource_Type.DWS_Table.value,
      tableCols: [
        'uuid',
        'name',
        'clusterOrHostName',
        'database',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'isAllowRestore',
        'operation'
      ],
      tableOpts: this.optItem
    };
  }

  onChange() {
    this.instanceDatabaseComponent.ngOnInit();
  }
}
