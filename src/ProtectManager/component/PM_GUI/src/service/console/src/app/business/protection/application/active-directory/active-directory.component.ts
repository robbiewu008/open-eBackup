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
import { DataMap } from 'app/shared';
import { RegisterComponent } from './register/register.component';

@Component({
  selector: 'aui-active-directory',
  templateUrl: './active-directory.component.html',
  styleUrls: ['./active-directory.component.less']
})
export class ActiveDirectoryComponent implements OnInit {
  config;
  constructor() {}

  ngOnInit(): void {
    this.config = {
      activeIndex: DataMap.Resource_Type.ActiveDirectory.value,
      tableCols: [
        'uuid',
        'name',
        'environmentName',
        'environmentEndpoint',
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
        'modify',
        'deleteResource'
      ],
      registerComponent: RegisterComponent
    };
  }
}
