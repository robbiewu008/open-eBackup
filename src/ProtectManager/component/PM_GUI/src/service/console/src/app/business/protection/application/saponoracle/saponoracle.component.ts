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
import { DatabaseTemplateComponent } from '../../host-app/database-template/database-template.component';
import { SaponoracleRegisterDatabaseComponent } from './register-database/register-database.component';
@Component({
  selector: 'aui-saponoracle',
  templateUrl: './saponoracle.component.html',
  styleUrls: ['./saponoracle.component.less']
})
export class SaponoracleComponent implements OnInit {
  activeIndex = 0;
  databaseConfig;
  @ViewChild(DatabaseTemplateComponent, { static: false })
  databaseTemplateComponent: DatabaseTemplateComponent;
  constructor() {}

  ngOnInit() {
    this.databaseConfig = {
      activeIndex: DataMap.Resource_Type.saponoracleDatabase.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'sapsid',
        'oraclesid',
        'sla_name',
        'oracle_version',
        'brtools_version',
        'osType',
        'environmentEndpoint',
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
      registerComponent: SaponoracleRegisterDatabaseComponent
    };
  }
  onChange() {
    this.databaseTemplateComponent.ngOnInit();
  }
}
