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
import { DatabaseTemplateComponent } from '../../database-template/database-template.component';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-email',
  templateUrl: './email.component.html',
  styleUrls: ['./email.component.less']
})
export class EmailComponent implements OnInit {
  instanceConfig;

  @ViewChild(DatabaseTemplateComponent, { static: false })
  databaseTemplateComponent: DatabaseTemplateComponent;

  constructor() {}

  ngOnInit(): void {
    this.instanceConfig = {
      activeIndex: DataMap.Resource_Type.ExchangeEmail.value,
      tableCols: [
        'uuid',
        'name',
        'address',
        'environmentName',
        'parentName',
        'sla_name',
        'sla_compliance',
        'sla_status',
        'protectionStatus',
        'operation'
      ],
      tableOpts: [
        'protect',
        'modifyProtect',
        'removeProtection',
        'activeProtection',
        'deactiveProtection',
        'recovery',
        'manualBackup'
      ]
    };
  }
}
