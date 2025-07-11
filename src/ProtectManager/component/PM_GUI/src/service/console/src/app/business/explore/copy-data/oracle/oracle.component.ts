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
  selector: 'aui-oracle',
  templateUrl: './oracle.component.html',
  styleUrls: ['./oracle.component.less']
})
export class OracleComponent implements OnInit {
  header = this.i18n.get('common_oracle_label');
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.oracle.value,
    DataMap.Resource_Type.oracleCluster.value,
    DataMap.Resource_Type.oraclePDB.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit() {}
}
