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
import { Component, OnInit, Output, EventEmitter } from '@angular/core';

@Component({
  selector: 'aui-job-filter',
  templateUrl: './job-filter.component.html',
  styleUrls: ['./job-filter.component.less']
})
export class JobFilterComponent implements OnInit {
  @Output() change = new EventEmitter<any>();

  constructor() {}

  ngOnInit() {}

  refresh() {
    this.change.emit();
  }
}
