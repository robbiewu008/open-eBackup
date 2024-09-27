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
import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import { DataMap } from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';

@Component({
  selector: 'aui-virtualization-group',
  templateUrl: './virtualization-group.component.html',
  styleUrls: ['./virtualization-group.component.less']
})
export class VirtualizationGroupComponent implements OnInit {
  @Input() treeSelection;
  @Input() subUnitType;
  @Output() updateTable = new EventEmitter();
  subType = DataMap.Resource_Type.vmGroup.value;

  header = '';
  activeIndex = '';
  treeData = [];
  expandedNodeList = [];

  optsConfig: ProButton[] = [];

  extParams = {};
  constructor() {}
  ngOnInit() {}
  updateTable1(e) {
    this.updateTable.emit({ total: e.total });
  }
}
