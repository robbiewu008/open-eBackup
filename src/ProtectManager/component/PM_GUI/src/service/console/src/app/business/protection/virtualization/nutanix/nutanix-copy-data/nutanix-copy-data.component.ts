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
import { Component } from '@angular/core';

@Component({
  selector: 'aui-nutanix-copy-data',
  templateUrl: './nutanix-copy-data.component.html',
  styleUrls: ['./nutanix-copy-data.component.less']
})
export class NutanixCopyDataComponent {
  data;
  type;

  constructor() {}

  ngOnInit(): void {}

  initDetailData(data) {
    this.data = data;
    this.type = data.resourceType;
  }
}
