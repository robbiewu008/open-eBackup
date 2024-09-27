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
import { RegisterGroupComponent } from './register-group/register-group.component';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-availabilty-group',
  templateUrl: './availabilty-group.component.html',
  styleUrls: ['./availabilty-group.component.less']
})
export class AvailabiltyGroupComponent implements OnInit {
  exchangeGroupOptions;

  @ViewChild(DatabaseTemplateComponent, { static: false })
  databaseTemplateComponent: DatabaseTemplateComponent;

  constructor() {}

  ngOnInit(): void {
    this.exchangeGroupOptions = {
      activeIndex: DataMap.Resource_Type.Exchange.value,
      tableCols: [
        'uuid',
        'name',
        'linkStatus',
        'subType',
        'sla_name',
        'sla_compliance',
        'sla_status',
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
        'rescan',
        'connectivityTest',
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterGroupComponent
    };
  }
}
