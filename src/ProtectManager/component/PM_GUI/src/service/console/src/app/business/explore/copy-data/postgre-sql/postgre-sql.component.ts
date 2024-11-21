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
import { ResourceType, DataMap } from 'app/shared';

@Component({
  selector: 'aui-postgre-sql',
  templateUrl: './postgre-sql.component.html',
  styleUrls: ['./postgre-sql.component.less']
})
export class PostgreSQLComponent implements OnInit {
  header = 'PostgreSQL';
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.PostgreSQLInstance.value,
    DataMap.Resource_Type.PostgreSQLClusterInstance.value
  ];

  constructor() {}

  ngOnInit(): void {}
}
