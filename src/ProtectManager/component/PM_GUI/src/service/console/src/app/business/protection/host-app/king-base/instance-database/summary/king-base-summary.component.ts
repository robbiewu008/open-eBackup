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
import { KingBaseProxyHostComponent } from './proxy-host/king-base-proxy-host.component';
@Component({
  selector: 'aui-king-base-summary',
  templateUrl: './king-base-summary.component.html',
  styleUrls: ['./king-base-summary.component.less']
})
export class KingBaseSummaryComponent implements OnInit {
  source;
  subType;
  dataMap = DataMap;
  constructor() {}

  @ViewChild(KingBaseProxyHostComponent, { static: false })
  instanceDatabaseComponent: KingBaseProxyHostComponent;
  ngOnInit() {}

  initDetailData(data) {
    this.source = data;
    this.subType = data.sub_type || data?.subType;
  }

  update() {
    this.instanceDatabaseComponent.refresh();
  }
}
