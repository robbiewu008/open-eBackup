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

@Component({
  selector: 'aui-oracle-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  targetName = 'Oracle01';
  mountTo = 'OrinalLoacation';
  location = 'Host001\\Oracle01';
  copyData = '2019/10/31 08:30AM';
  maskEngine = 'Mask in Local Cluster';
  scriptTool = 'Desensitizing Tool of Finacial Department.py (168k)';
  constructor() {}

  ngOnInit() {}
}
