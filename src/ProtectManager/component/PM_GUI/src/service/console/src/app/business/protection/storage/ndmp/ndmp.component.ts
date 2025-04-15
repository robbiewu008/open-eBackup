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
import { CreateNdmpComponent } from './create-ndmp/create-ndmp.component';

@Component({
  selector: 'aui-ndmp',
  templateUrl: './ndmp.component.html',
  styleUrls: ['./ndmp.component.less']
})
export class NdmpComponent implements OnInit {
  databaseConfig;
  activeIndex = 'fileSystem';
  subType = DataMap.Resource_Type.ndmp.value;
  ngOnInit(): void {
    this.databaseConfig = {
      activeIndex: DataMap.Resource_Type.ndmp.value,
      tableCols: [
        'uuid',
        'name',
        'directory',
        'parentName',
        'environmentName',
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
        'resourceAuth',
        'resourceReclaiming',
        'recovery',
        'manualBackup',
        'deleteResource'
      ],
      registerComponent: CreateNdmpComponent
    };
  }
}
