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
import { ResourceType, DataMap, I18NService } from 'app/shared';

@Component({
  selector: 'aui-mysql',
  templateUrl: './mysql.component.html',
  styleUrls: ['./mysql.component.less']
})
export class MysqlComponent implements OnInit {
  header = this.i18n.get('protection_mysql_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.MySQLInstance.value,
    DataMap.Resource_Type.MySQLClusterInstance.value,
    DataMap.Resource_Type.MySQLDatabase.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
