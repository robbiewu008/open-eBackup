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
  selector: 'aui-gaussdb-t',
  templateUrl: './gaussdb-t.component.html',
  styleUrls: ['./gaussdb-t.component.less']
})
export class GaussdbTComponent implements OnInit {
  header = 'GaussDB T';
  resourceType = ResourceType.DATABASE;
  childResourceType = [
    DataMap.Resource_Type.GaussDB_T.value,
    DataMap.Resource_Type.gaussdbTSingle.value
  ];

  constructor(private i18n: I18NService) {}

  ngOnInit() {}
}
