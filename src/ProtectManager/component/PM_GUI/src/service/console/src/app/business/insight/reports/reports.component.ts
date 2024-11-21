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
import { Component, OnDestroy, OnInit, ViewChild } from '@angular/core';
import { ReportsListComponent } from './reports-list/reports-list.component';

@Component({
  selector: 'aui-reports',
  templateUrl: './reports.component.html',
  styleUrls: ['./reports.component.less']
})
export class ReportsComponent implements OnInit, OnDestroy {
  @ViewChild(ReportsListComponent, { static: false })
  reportsListComponent: ReportsListComponent;
  constructor() {}

  ngOnDestroy() {}

  ngOnInit() {}

  onChange() {
    this.reportsListComponent.ngOnInit();
    this.reportsListComponent.ngAfterViewInit();
  }
}
